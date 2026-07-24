#pragma once

#include <qrcode/bit_grid.h>
#include <qrcode/function_patterns.h>
#include <qrcode/mask_pattern.h>

#include <cstdint>
#include <vector>

namespace qr {

/// Reads and unmasks codeword bits in QR zig-zag order.
/// @param grid Version 1-4 module grid.
/// @param function_patterns Map of modules that do not contain codeword data.
/// @param mask Mask selected by the QR format information.
/// @return Unmasked codeword bits, excluding remainder bits.
/// @throws std::invalid_argument if the grid dimensions are unsupported.
[[nodiscard]] std::vector<std::uint8_t> read_data_bits(
    const BitGrid& grid,
    const FunctionPatternMap& function_patterns,
    MaskPattern mask
);

} // namespace qr
