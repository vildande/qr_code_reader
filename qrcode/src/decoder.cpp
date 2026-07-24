#include <qrcode/decoder.h>

#include <qrcode/bit_stream.h>
#include <qrcode/data_path.h>
#include <qrcode/mask_pattern.h>
#include <qrcode/segment_header.h>

#include <CharacterSet.h>
#include <TextDecoder.h>

#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace qr {
namespace {

struct BlockLayout {
    std::size_t block_count;
    std::size_t data_codewords_per_block;
};

struct FormatInfo {
    MaskPattern mask;
    std::size_t layout_index;
};

struct GaloisTables {
    std::array<std::uint8_t, 512> exponent{};
    std::array<std::uint8_t, 256> logarithm{};
};

constexpr std::array<std::array<BlockLayout, 4>, 4> block_layouts{{
    {{{1, 19}, {1, 16}, {1, 13}, {1, 9}}},
    {{{1, 34}, {1, 28}, {1, 22}, {1, 16}}},
    {{{1, 55}, {1, 44}, {2, 17}, {2, 13}}},
    {{{1, 80}, {2, 32}, {2, 24}, {4, 9}}},
}};

constexpr GaloisTables make_galois_tables() {
    GaloisTables tables;
    unsigned int value = 1;

    for (std::size_t exponent = 0; exponent < 255; ++exponent) {
        tables.exponent[exponent] = static_cast<std::uint8_t>(value);
        tables.logarithm[value] = static_cast<std::uint8_t>(exponent);
        value <<= 1U;
        if ((value & 0x100U) != 0) {
            value ^= 0x11DU;
        }
    }
    for (std::size_t exponent = 255; exponent < tables.exponent.size(); ++exponent) {
        tables.exponent[exponent] = tables.exponent[exponent - 255];
    }

    return tables;
}

constexpr auto galois = make_galois_tables();

constexpr std::uint8_t gf_multiply(std::uint8_t left, std::uint8_t right) noexcept {
    if (left == 0 || right == 0) {
        return 0;
    }

    return galois.exponent[
        static_cast<std::size_t>(galois.logarithm[left]) + galois.logarithm[right]
    ];
}

constexpr std::uint8_t gf_inverse(std::uint8_t value) {
    if (value == 0) {
        throw std::invalid_argument{"cannot invert zero in GF(256)"};
    }
    return galois.exponent[255U - galois.logarithm[value]];
}

std::uint16_t make_format_pattern(std::uint8_t data) {
    constexpr std::uint16_t generator = 0x537;
    constexpr std::uint16_t format_mask = 0x5412;

    std::uint16_t remainder = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(data) << 10U
    );
    for (int bit = 14; bit >= 10; --bit) {
        if ((remainder & (1U << bit)) != 0) {
            remainder ^= static_cast<std::uint16_t>(generator << (bit - 10));
        }
    }

    return static_cast<std::uint16_t>(
        ((static_cast<std::uint16_t>(data) << 10U) | remainder) ^ format_mask
    );
}

FormatInfo read_format_info(const BitGrid& grid) {
    std::uint16_t first{};
    std::uint16_t second{};

    auto append = [&grid](std::uint16_t& bits, ModuleCoord position) {
        bits = static_cast<std::uint16_t>(
            (bits << 1U) | static_cast<std::uint16_t>(grid.at(position))
        );
    };

    for (int x = 0; x < 6; ++x) {
        append(first, {x, 8});
    }
    append(first, {7, 8});
    append(first, {8, 8});
    append(first, {8, 7});
    for (int y = 5; y >= 0; --y) {
        append(first, {8, y});
    }

    const int size = grid.width();
    for (int y = size - 1; y >= size - 7; --y) {
        append(second, {8, y});
    }
    for (int x = size - 8; x < size; ++x) {
        append(second, {x, 8});
    }

    int best_distance = 16;
    int best_data = -1;
    bool ambiguous = false;

    for (int data = 0; data < 32; ++data) {
        const auto pattern = make_format_pattern(static_cast<std::uint8_t>(data));
        const int distance = std::min(
            std::popcount(static_cast<unsigned int>(pattern ^ first)),
            std::popcount(static_cast<unsigned int>(pattern ^ second))
        );

        if (distance < best_distance) {
            best_distance = distance;
            best_data = data;
            ambiguous = false;
        } else if (distance == best_distance && data != best_data) {
            ambiguous = true;
        }
    }

    if (best_distance > 3 || ambiguous) {
        throw std::invalid_argument{"invalid or ambiguous QR format information"};
    }

    constexpr std::array<std::size_t, 4> layout_for_ec_bits{1, 0, 3, 2};
    const auto ec_bits = static_cast<std::size_t>((best_data >> 3) & 0b11);

    return FormatInfo{
        .mask = static_cast<MaskPattern>(best_data & 0b111),
        .layout_index = layout_for_ec_bits[ec_bits],
    };
}

std::vector<std::uint8_t> calculate_syndromes(
    const std::vector<std::uint8_t>& codewords,
    std::size_t ec_codewords
) {
    std::vector<std::uint8_t> syndromes(ec_codewords);

    for (std::size_t index = 0; index < ec_codewords; ++index) {
        const auto point = galois.exponent[index];
        std::uint8_t value{};
        for (const auto codeword : codewords) {
            value = static_cast<std::uint8_t>(gf_multiply(value, point) ^ codeword);
        }
        syndromes[index] = value;
    }

    return syndromes;
}

std::vector<std::uint8_t> correct_version1_data_bits(
    const std::vector<std::uint8_t>& raw_bits,
    std::size_t data_codewords
) {
    constexpr std::size_t bits_per_codeword = 8;
    constexpr std::size_t total_codewords = 26;

    if (raw_bits.size() != total_codewords * bits_per_codeword ||
        data_codewords >= total_codewords) {
        throw std::invalid_argument{"invalid Version 1 QR block"};
    }

    std::vector<std::uint8_t> codewords(total_codewords);
    for (std::size_t index = 0; index < total_codewords; ++index) {
        for (std::size_t bit = 0; bit < bits_per_codeword; ++bit) {
            codewords[index] = static_cast<std::uint8_t>(
                (codewords[index] << 1U) | raw_bits[index * bits_per_codeword + bit]
            );
        }
    }

    const std::size_t ec_codewords = total_codewords - data_codewords;
    const auto syndromes = calculate_syndromes(codewords, ec_codewords);

    if (!std::ranges::all_of(syndromes, [](std::uint8_t value) { return value == 0; })) {
        // Berlekamp-Massey builds the error-locator polynomial.
        std::vector<std::uint8_t> locator(ec_codewords + 1);
        std::vector<std::uint8_t> previous_locator(ec_codewords + 1);
        locator[0] = 1;
        previous_locator[0] = 1;

        std::size_t degree{};
        std::size_t shift = 1;
        std::uint8_t previous_discrepancy = 1;

        for (std::size_t step = 0; step < ec_codewords; ++step) {
            std::uint8_t discrepancy = syndromes[step];
            for (std::size_t index = 1; index <= degree; ++index) {
                discrepancy ^= gf_multiply(locator[index], syndromes[step - index]);
            }

            if (discrepancy == 0) {
                ++shift;
                continue;
            }

            const auto old_locator = locator;
            const auto scale = gf_multiply(discrepancy, gf_inverse(previous_discrepancy));
            for (std::size_t index = 0; index + shift < locator.size(); ++index) {
                locator[index + shift] ^= gf_multiply(scale, previous_locator[index]);
            }

            if (2 * degree <= step) {
                degree = step + 1 - degree;
                previous_locator = old_locator;
                previous_discrepancy = discrepancy;
                shift = 1;
            } else {
                ++shift;
            }
        }

        if (degree == 0 || 2 * degree > ec_codewords) {
            throw std::invalid_argument{"uncorrectable QR data"};
        }

        auto evaluate = [](const std::vector<std::uint8_t>& polynomial, std::uint8_t point) {
            std::uint8_t value{};
            for (auto coefficient = polynomial.rbegin(); coefficient != polynomial.rend();
                 ++coefficient) {
                value = static_cast<std::uint8_t>(
                    gf_multiply(value, point) ^ *coefficient
                );
            }
            return value;
        };

        std::vector<std::size_t> error_positions;
        std::vector<std::uint8_t> error_locations;
        error_positions.reserve(degree);
        error_locations.reserve(degree);

        // Chien search maps the locator roots back to QR codeword positions.
        for (std::size_t position = 0; position < codewords.size(); ++position) {
            const std::size_t exponent = codewords.size() - 1 - position;
            const auto location = galois.exponent[exponent];
            if (evaluate(locator, gf_inverse(location)) == 0) {
                error_positions.push_back(position);
                error_locations.push_back(location);
            }
        }

        if (error_positions.size() != degree) {
            throw std::invalid_argument{"uncorrectable QR data"};
        }

        std::vector<std::uint8_t> evaluator(ec_codewords);
        for (std::size_t locator_index = 0; locator_index <= degree; ++locator_index) {
            for (std::size_t syndrome_index = 0;
                 locator_index + syndrome_index < evaluator.size(); ++syndrome_index) {
                evaluator[locator_index + syndrome_index] ^=
                    gf_multiply(locator[locator_index], syndromes[syndrome_index]);
            }
        }

        // Forney's formula gives the correction magnitude at each position.
        for (std::size_t index = 0; index < error_positions.size(); ++index) {
            const auto inverse_location = gf_inverse(error_locations[index]);
            std::uint8_t denominator = 1;

            for (std::size_t other = 0; other < error_locations.size(); ++other) {
                if (other != index) {
                    denominator = gf_multiply(
                        denominator,
                        static_cast<std::uint8_t>(
                            1U ^ gf_multiply(error_locations[other], inverse_location)
                        )
                    );
                }
            }

            const auto magnitude = gf_multiply(
                evaluate(evaluator, inverse_location),
                gf_inverse(denominator)
            );
            codewords[error_positions[index]] ^= magnitude;
        }

        const auto corrected_syndromes = calculate_syndromes(codewords, ec_codewords);
        if (!std::ranges::all_of(
                corrected_syndromes,
                [](std::uint8_t value) { return value == 0; }
            )) {
            throw std::invalid_argument{"uncorrectable QR data"};
        }
    }

    std::vector<std::uint8_t> data_bits;
    data_bits.reserve(data_codewords * bits_per_codeword);
    for (std::size_t index = 0; index < data_codewords; ++index) {
        for (int bit = 7; bit >= 0; --bit) {
            data_bits.push_back(static_cast<std::uint8_t>((codewords[index] >> bit) & 1U));
        }
    }
    return data_bits;
}

std::vector<std::uint8_t> deinterleave_data_bits(
    const std::vector<std::uint8_t>& raw_bits,
    BlockLayout layout
) {
    constexpr std::size_t bits_per_codeword = 8;
    const std::size_t data_codewords = layout.block_count * layout.data_codewords_per_block;

    if (data_codewords * bits_per_codeword > raw_bits.size()) {
        throw std::invalid_argument{"invalid QR block layout"};
    }

    std::vector<std::uint8_t> data_bits;
    data_bits.reserve(data_codewords * bits_per_codeword);

    for (std::size_t block = 0; block < layout.block_count; ++block) {
        for (std::size_t index = 0; index < layout.data_codewords_per_block; ++index) {
            const std::size_t raw_codeword = index * layout.block_count + block;
            for (std::size_t bit = 0; bit < bits_per_codeword; ++bit) {
                data_bits.push_back(raw_bits[raw_codeword * bits_per_codeword + bit]);
            }
        }
    }

    return data_bits;
}

std::string decode_numeric(BitStream& stream, std::size_t character_count) {
    std::string result;
    result.reserve(character_count);

    while (character_count >= 3) {
        const auto value = stream.read_bits(10);
        if (value >= 1000) {
            throw std::invalid_argument{"invalid numeric QR group"};
        }

        result.push_back(static_cast<char>('0' + value / 100));
        result.push_back(static_cast<char>('0' + value / 10 % 10));
        result.push_back(static_cast<char>('0' + value % 10));
        character_count -= 3;
    }

    if (character_count == 2) {
        const auto value = stream.read_bits(7);
        if (value >= 100) {
            throw std::invalid_argument{"invalid numeric QR group"};
        }
        result.push_back(static_cast<char>('0' + value / 10));
        result.push_back(static_cast<char>('0' + value % 10));
    } else if (character_count == 1) {
        const auto value = stream.read_bits(4);
        if (value >= 10) {
            throw std::invalid_argument{"invalid numeric QR group"};
        }
        result.push_back(static_cast<char>('0' + value));
    }

    return result;
}

std::string decode_alphanumeric(BitStream& stream, std::size_t character_count) {
    constexpr std::string_view characters{"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:"};

    std::string result;
    result.reserve(character_count);

    while (character_count >= 2) {
        const auto value = stream.read_bits(11);
        if (value >= characters.size() * characters.size()) {
            throw std::invalid_argument{"invalid alphanumeric QR group"};
        }

        result.push_back(characters[value / characters.size()]);
        result.push_back(characters[value % characters.size()]);
        character_count -= 2;
    }

    if (character_count == 1) {
        const auto value = stream.read_bits(6);
        if (value >= characters.size()) {
            throw std::invalid_argument{"invalid alphanumeric QR character"};
        }
        result.push_back(characters[value]);
    }

    return result;
}

std::string decode_bytes(BitStream& stream, std::size_t character_count) {
    std::string result;
    result.reserve(character_count);

    for (std::size_t i = 0; i < character_count; ++i) {
        result.push_back(static_cast<char>(stream.read_bits(8)));
    }

    return result;
}

std::string decode_kanji(BitStream& stream, std::size_t character_count) {
    constexpr std::uint32_t kanji_base = 0xC0;

    std::vector<std::uint8_t> shift_jis;
    shift_jis.reserve(character_count * 2);

    for (std::size_t i = 0; i < character_count; ++i) {
        const auto value = stream.read_bits(13);
        const auto assembled = ((value / kanji_base) << 8U) | (value % kanji_base);
        const auto code = assembled < 0x1F00U ? assembled + 0x8140U : assembled + 0xC140U;
        const auto lead = code >> 8U;
        const auto trail = code & 0xFFU;

        const bool valid_range = (code >= 0x8140U && code <= 0x9FFCU) ||
                                 (code >= 0xE040U && code <= 0xEBBFU);
        const bool valid_lead = (lead >= 0x81U && lead <= 0x9FU) ||
                                (lead >= 0xE0U && lead <= 0xEBU);
        const bool valid_trail = (trail >= 0x40U && trail <= 0x7EU) ||
                                 (trail >= 0x80U && trail <= 0xFCU);
        if (!valid_range || !valid_lead || !valid_trail) {
            throw std::invalid_argument{"invalid QR Kanji value"};
        }

        shift_jis.push_back(static_cast<std::uint8_t>(lead));
        shift_jis.push_back(static_cast<std::uint8_t>(trail));
    }

    auto result = ZXing::BytesToUtf8(shift_jis, ZXing::CharacterSet::Shift_JIS);
    if (result.empty()) {
        throw std::invalid_argument{"invalid Shift JIS QR text"};
    }
    return result;
}

std::string decode_candidate(const std::vector<std::uint8_t>& bits) {
    BitStream stream{bits};
    std::string result;

    while (true) {
        const auto header = read_segment_header(stream);
        if (header.mode == Mode::terminator) {
            if (result.empty()) {
                throw std::invalid_argument{"empty QR message"};
            }

            const std::size_t alignment_bits = stream.remaining() % 8;
            if (stream.read_bits(alignment_bits) != 0) {
                throw std::invalid_argument{"invalid QR alignment padding"};
            }

            constexpr std::array<std::uint32_t, 2> pad_codewords{0xEC, 0x11};
            std::size_t pad_index{};
            while (stream.remaining() > 0) {
                if (stream.read_bits(8) != pad_codewords[pad_index % pad_codewords.size()]) {
                    throw std::invalid_argument{"invalid QR pad codeword"};
                }
                ++pad_index;
            }

            return result;
        }
        if (header.character_count == 0) {
            throw std::invalid_argument{"empty QR segment"};
        }

        switch (header.mode) {
        case Mode::numeric:
            result += decode_numeric(stream, header.character_count);
            break;
        case Mode::alphanumeric:
            result += decode_alphanumeric(stream, header.character_count);
            break;
        case Mode::byte:
            result += decode_bytes(stream, header.character_count);
            break;
        case Mode::kanji:
            result += decode_kanji(stream, header.character_count);
            break;
        case Mode::terminator:
            break;
        }
    }
}

} // namespace

std::string decode_message(
    const BitGrid& grid,
    const FunctionPatternMap& function_patterns
) {
    const int size = grid.width();
    if (size != grid.height() || size < 21 || size > 33 || (size - 17) % 4 != 0) {
        throw std::invalid_argument{"decode_message supports only QR versions 1-4"};
    }

    const int version = (size - 17) / 4;
    const auto format = read_format_info(grid);
    const auto& layouts = block_layouts[static_cast<std::size_t>(version - 1)];
    const auto raw_bits = read_data_bits(grid, function_patterns, format.mask);
    const auto layout = layouts[format.layout_index];
    const auto data_bits = version == 1
        ? correct_version1_data_bits(raw_bits, layout.data_codewords_per_block)
        : deinterleave_data_bits(raw_bits, layout);
    return decode_candidate(data_bits);
}

} // namespace qr
