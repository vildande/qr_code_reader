#include <qrcode/data_path.h>

#include <array>
#include <cstddef>
#include <stdexcept>

namespace qr {

std::vector<std::uint8_t> read_data_bits(
    const BitGrid& grid,
    const FunctionPatternMap& function_patterns,
    MaskPattern mask
) {
    constexpr int minimum_size = 21;
    constexpr int maximum_size = 33;
    constexpr int base_size = 17;
    constexpr int version_step = 4;
    constexpr std::array<std::size_t, 4> codeword_bits{208, 352, 560, 800};

    const int size = grid.width();
    if (size != grid.height() || size < minimum_size || size > maximum_size ||
        (size - base_size) % version_step != 0) {
        throw std::invalid_argument{"read_data_bits supports only square QR version 1-4 grids"};
    }

    const int version = (size - base_size) / version_step;
    const std::size_t expected_bits = codeword_bits[static_cast<std::size_t>(version - 1)];
    const std::size_t remainder_bits = version == 1 ? 0 : 7;

    std::vector<std::uint8_t> bits;
    bits.reserve(expected_bits + remainder_bits);

    bool upward = true;
    for (int right_x = size - 1; right_x > 0; right_x -= 2) {
        if (right_x == 6) {
            --right_x;
        }

        for (int step = 0; step < size; ++step) {
            const int y = upward ? size - 1 - step : step;

            for (int dx = 0; dx < 2; ++dx) {
                const ModuleCoord p{right_x - dx, y};
                if (function_patterns.is_function_pattern(p)) {
                    continue;
                }

                const bool bit = grid.at(p) ^ mask_applies(mask, p);
                bits.push_back(static_cast<std::uint8_t>(bit));
            }
        }

        upward = !upward;
    }

    if (bits.size() != expected_bits + remainder_bits) {
        throw std::runtime_error{"unexpected number of QR data modules"};
    }
    bits.resize(expected_bits);

    return bits;
}

} // namespace qr
