#include <qrcode/bit_stream.h>
#include <qrcode/segment_header.h>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace {

void append_bits(std::vector<std::uint8_t>& bits, std::uint32_t value, std::size_t count) {
    for (std::size_t bit = count; bit > 0; --bit) {
        bits.push_back(static_cast<std::uint8_t>((value >> (bit - 1)) & 1U));
    }
}

} // namespace

TEST_CASE("Segment headers use Version 1-9 character-count widths", "[segment]") {
    struct HeaderCase {
        qr::Mode mode;
        std::size_t count_bits;
        std::uint32_t count;
    };
    constexpr std::array cases{
        HeaderCase{qr::Mode::numeric, 10, 713},
        HeaderCase{qr::Mode::alphanumeric, 9, 301},
        HeaderCase{qr::Mode::byte, 8, 201},
        HeaderCase{qr::Mode::kanji, 8, 83},
    };

    for (const auto test : cases) {
        std::vector<std::uint8_t> bits;
        append_bits(bits, static_cast<std::uint8_t>(test.mode), 4);
        append_bits(bits, test.count, test.count_bits);
        qr::BitStream stream{bits};

        const auto header = qr::read_segment_header(stream);
        CAPTURE(test.count_bits, test.count);
        CHECK(header.mode == test.mode);
        CHECK(header.character_count == test.count);
        CHECK(stream.remaining() == 0);
    }
}

TEST_CASE("Segment header handles terminators and rejects unsupported modes", "[segment]") {
    constexpr std::array<std::uint8_t, 4> terminator_bits{0, 0, 0, 0};
    qr::BitStream terminator_stream{terminator_bits};
    const auto terminator = qr::read_segment_header(terminator_stream);
    CHECK(terminator.mode == qr::Mode::terminator);
    CHECK(terminator.character_count == 0);

    constexpr std::array<std::uint8_t, 4> unsupported_bits{0, 0, 1, 1};
    qr::BitStream unsupported_stream{unsupported_bits};
    CHECK_THROWS_AS(qr::read_segment_header(unsupported_stream), std::invalid_argument);

    constexpr std::array<std::uint8_t, 4> numeric_without_count{0, 0, 0, 1};
    qr::BitStream incomplete_stream{numeric_without_count};
    CHECK_THROWS_AS(qr::read_segment_header(incomplete_stream), std::out_of_range);
}
