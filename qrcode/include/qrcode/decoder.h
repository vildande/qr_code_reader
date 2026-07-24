#pragma once

#include <qrcode/bit_grid.h>
#include <qrcode/function_patterns.h>

#include <string>

namespace qr {

/// Decodes a complete Model 2 Version 1-4 QR module grid.
/// @param grid Binarized QR modules without the quiet zone.
/// @param function_patterns Reserved-module map for the same version.
/// @return Decoded UTF-8 text.
/// @throws std::invalid_argument if QR metadata or codewords are invalid.
/// @throws std::out_of_range if a segment is truncated.
[[nodiscard]] std::string decode_message(
    const BitGrid& grid,
    const FunctionPatternMap& function_patterns
);

} // namespace qr
