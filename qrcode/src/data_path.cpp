#include <qrcode/data_path.h>

#include <stdexcept>

namespace qr {

std::vector<std::uint8_t> read_data_bits(
    const BitGrid& grid,
    const FunctionPatternMap& function_patterns
) {
    constexpr int version1_size = 21;

    if (grid.width() != version1_size || grid.height() != version1_size) {
        throw std::invalid_argument{"read_data_bits currently supports only 21x21 QR version 1 grids"};
    }

    std::vector<std::uint8_t> bits;
    bits.reserve(208);

    bool upward = true;
    for (int right_x = version1_size - 1; right_x > 0; right_x -= 2) {
        if (right_x == 6) {
            --right_x;
        }

        for (int step = 0; step < version1_size; ++step) {
            const int y = upward ? version1_size - 1 - step : step;

            for (int dx = 0; dx < 2; ++dx) {
                const ModuleCoord p{right_x - dx, y};
                if (function_patterns.is_function_pattern(p)) {
                    continue;
                }

                bits.push_back(static_cast<std::uint8_t>(grid.at(p)));
            }
        }

        upward = !upward;
    }

    return bits;
}

} // namespace qr
