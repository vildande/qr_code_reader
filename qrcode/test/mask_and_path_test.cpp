#include <qrcode/bit_grid.h>
#include <qrcode/data_path.h>
#include <qrcode/function_patterns.h>
#include <qrcode/mask_pattern.h>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace {

bool expected_mask(qr::MaskPattern mask, qr::ModuleCoord position) {
    const int x = position.x;
    const int y = position.y;
    const int product = x * y;

    switch (mask) {
    case qr::MaskPattern::pattern0:
        return (x + y) % 2 == 0;
    case qr::MaskPattern::pattern1:
        return y % 2 == 0;
    case qr::MaskPattern::pattern2:
        return x % 3 == 0;
    case qr::MaskPattern::pattern3:
        return (x + y) % 3 == 0;
    case qr::MaskPattern::pattern4:
        return (y / 2 + x / 3) % 2 == 0;
    case qr::MaskPattern::pattern5:
        return product % 2 + product % 3 == 0;
    case qr::MaskPattern::pattern6:
        return (product % 2 + product % 3) % 2 == 0;
    case qr::MaskPattern::pattern7:
        return ((x + y) % 2 + product % 3) % 2 == 0;
    }
    return false;
}

} // namespace

TEST_CASE("All eight QR mask formulas match the specification", "[mask]") {
    constexpr std::array masks{
        qr::MaskPattern::pattern0, qr::MaskPattern::pattern1,
        qr::MaskPattern::pattern2, qr::MaskPattern::pattern3,
        qr::MaskPattern::pattern4, qr::MaskPattern::pattern5,
        qr::MaskPattern::pattern6, qr::MaskPattern::pattern7,
    };

    for (const auto mask : masks) {
        for (int y = 0; y < 33; ++y) {
            for (int x = 0; x < 33; ++x) {
                CAPTURE(mask, x, y);
                CHECK(qr::mask_applies(mask, {x, y}) == expected_mask(mask, {x, y}));
            }
        }
    }

    CHECK_THROWS_AS(
        qr::mask_applies(static_cast<qr::MaskPattern>(8), {0, 0}),
        std::invalid_argument
    );
}

TEST_CASE("Data traversal skips patterns and removes masks for Versions 1-4", "[data-path]") {
    constexpr std::array<std::size_t, 4> expected_sizes{208, 352, 560, 800};

    for (int version = 1; version <= 4; ++version) {
        const int size = 17 + 4 * version;
        qr::BitGrid grid{size, size};
        const qr::FunctionPatternMap patterns{version};

        for (int y = 0; y < size; ++y) {
            for (int x = 0; x < size; ++x) {
                const qr::ModuleCoord position{x, y};
                if (!patterns.is_function_pattern(position)) {
                    grid.set(position, qr::mask_applies(qr::MaskPattern::pattern6, position));
                }
            }
        }

        const auto bits = qr::read_data_bits(grid, patterns, qr::MaskPattern::pattern6);
        CAPTURE(version);
        CHECK(bits.size() == expected_sizes[static_cast<std::size_t>(version - 1)]);
        CHECK(std::ranges::all_of(bits, [](std::uint8_t bit) { return bit == 0; }));
    }
}

TEST_CASE("Version 1 traversal starts at the bottom-right pair", "[data-path]") {
    qr::BitGrid grid{21, 21};
    const qr::FunctionPatternMap patterns{1};

    for (int y = 0; y < 21; ++y) {
        for (int x = 0; x < 21; ++x) {
            const qr::ModuleCoord position{x, y};
            if (!patterns.is_function_pattern(position)) {
                grid.set(position, qr::mask_applies(qr::MaskPattern::pattern0, position));
            }
        }
    }
    grid.set({20, 20}, !grid.at({20, 20}));
    grid.set({19, 20}, !grid.at({19, 20}));

    const auto bits = qr::read_data_bits(grid, patterns, qr::MaskPattern::pattern0);
    REQUIRE(bits.size() == 208);
    CHECK(bits[0] == 1);
    CHECK(bits[1] == 1);
}

TEST_CASE("Data traversal rejects unsupported dimensions", "[data-path]") {
    const qr::FunctionPatternMap patterns{1};
    CHECK_THROWS_AS(
        qr::read_data_bits(qr::BitGrid{20, 21}, patterns, qr::MaskPattern::pattern0),
        std::invalid_argument
    );
}
