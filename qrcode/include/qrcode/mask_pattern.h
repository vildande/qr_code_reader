#pragma once

#include <qrcode/bit_grid.h>

#include <cstdint>

namespace qr {

/// The eight data-mask formulas defined by the QR specification.
enum class MaskPattern : std::uint8_t {
    pattern0, ///< '(x + y) mod 2 == 0'
    pattern1, ///< 'y mod 2 == 0'
    pattern2, ///< 'x mod 3 == 0'
    pattern3, ///< '(x + y) mod 3 == 0'
    pattern4, ///< '(floor(y / 2) + floor(x / 3)) mod 2 == 0'
    pattern5, ///< '(x*y mod 2) + (x*y mod 3) == 0'
    pattern6, ///< '((x*y mod 2) + (x*y mod 3)) mod 2 == 0'
    pattern7, ///< '((x+y mod 2) + (x*y mod 3)) mod 2 == 0'
};

/// Evaluates a mask formula at one module coordinate.
/// @throws std::invalid_argument if 'pattern' is not a valid enumerator.
[[nodiscard]] bool mask_applies(MaskPattern pattern, ModuleCoord position);

} // namespace qr
