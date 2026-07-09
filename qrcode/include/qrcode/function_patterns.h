#pragma once

#include <qrcode/bit_grid.h>

namespace qr {

class FunctionPatternMap {
public:
    explicit FunctionPatternMap(int version);

    [[nodiscard]] bool is_function_pattern(ModuleCoord p) const noexcept;

private:
    BitGrid reserved_;
};

} // namespace qr
