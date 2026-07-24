#pragma once

#include <qrcode/bit_grid.h>

namespace qr {

/// Identifies QR modules reserved for fixed patterns and metadata.
class FunctionPatternMap {
public:
    /// Builds the reserved map for a Model 2 QR version.
    /// @throws std::invalid_argument if 'version' is outside 1-4.
    explicit FunctionPatternMap(int version);

    /// Returns 'true' for reserved modules and 'false' outside the grid.
    [[nodiscard]] bool is_function_pattern(ModuleCoord p) const noexcept;

private:
    BitGrid reserved_;
};

} // namespace qr
