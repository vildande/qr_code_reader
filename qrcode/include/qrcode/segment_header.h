#pragma once

#include <qrcode/bit_stream.h>

#include <cstddef>
#include <cstdint>

namespace qr {

/// Segment modes supported by the decoder.
enum class Mode : std::uint8_t {
    terminator = 0b0000,   ///< End of encoded content.
    numeric = 0b0001,      ///< Decimal digits.
    alphanumeric = 0b0010, ///< QR's 45-character alphabet.
    byte = 0b0100,         ///< Eight-bit bytes.
    kanji = 0b1000,        ///< QR-encoded Shift JIS characters.
};

/// Parsed mode and character count for one segment.
struct SegmentHeader {
    Mode mode;                  ///< Segment encoding mode.
    std::size_t character_count; ///< Number of encoded characters or bytes.
};

/// Reads a Version 1-9 segment mode and character count.
/// @throws std::invalid_argument if the mode is unsupported.
/// @throws std::out_of_range if the header is truncated.
[[nodiscard]] SegmentHeader read_segment_header(BitStream& stream);

} // namespace qr
