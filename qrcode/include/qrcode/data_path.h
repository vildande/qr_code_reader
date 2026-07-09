#pragma once

#include <qrcode/bit_grid.h>
#include <qrcode/function_patterns.h>

#include <cstdint>
#include <vector>

namespace qr {

[[nodiscard]] std::vector<std::uint8_t> read_data_bits(
    const BitGrid& grid,
    const FunctionPatternMap& function_patterns
);

} // namespace qr
