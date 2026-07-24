#include <qrcode/mask_pattern.h>

#include <stdexcept>

namespace qr {

bool mask_applies(MaskPattern pattern, ModuleCoord position) {
    const int x = position.x;
    const int y = position.y;
    const int product = x * y;

    switch (pattern) {
    case MaskPattern::pattern0:
        return (x + y) % 2 == 0;
    case MaskPattern::pattern1:
        return y % 2 == 0;
    case MaskPattern::pattern2:
        return x % 3 == 0;
    case MaskPattern::pattern3:
        return (x + y) % 3 == 0;
    case MaskPattern::pattern4:
        return (y / 2 + x / 3) % 2 == 0;
    case MaskPattern::pattern5:
        return (product % 2) + (product % 3) == 0;
    case MaskPattern::pattern6:
        return ((product % 2) + (product % 3)) % 2 == 0;
    case MaskPattern::pattern7:
        return (((x + y) % 2) + (product % 3)) % 2 == 0;
    }

    throw std::invalid_argument{"invalid QR mask pattern"};
}

} // namespace qr
