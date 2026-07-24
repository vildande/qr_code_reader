#include <qrcode/segment_header.h>

#include <stdexcept>

namespace qr {

SegmentHeader read_segment_header(BitStream& stream) {
    constexpr std::size_t mode_bits = 4;
    constexpr std::size_t numeric_count_bits = 10;
    constexpr std::size_t alphanumeric_count_bits = 9;
    constexpr std::size_t byte_count_bits = 8;
    constexpr std::size_t kanji_count_bits = 8;

    const auto mode_value = stream.read_bits(mode_bits);
    const auto mode = static_cast<Mode>(mode_value);

    if (mode == Mode::terminator) {
        return SegmentHeader{.mode = mode, .character_count = 0};
    }

    std::size_t count_bits{};
    switch (mode) {
    case Mode::numeric:
        count_bits = numeric_count_bits;
        break;
    case Mode::alphanumeric:
        count_bits = alphanumeric_count_bits;
        break;
    case Mode::byte:
        count_bits = byte_count_bits;
        break;
    case Mode::kanji:
        count_bits = kanji_count_bits;
        break;
    default:
        throw std::invalid_argument{"unsupported QR mode"};
    }

    return SegmentHeader{
        .mode = mode,
        .character_count = stream.read_bits(count_bits),
    };
}

} // namespace qr
