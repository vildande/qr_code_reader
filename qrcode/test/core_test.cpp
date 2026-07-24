#include <qrcode/bit_grid.h>
#include <qrcode/bit_stream.h>
#include <qrcode/function_patterns.h>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>
#include <stdexcept>

TEST_CASE("BitGrid stores modules and checks coordinates", "[grid]") {
    qr::BitGrid grid{3, 2};

    CHECK(grid.width() == 3);
    CHECK(grid.height() == 2);
    CHECK(grid.in_bounds({2, 1}));
    CHECK_FALSE(grid.in_bounds({3, 1}));

    grid.set({2, 1}, true);
    CHECK(grid.at({2, 1}));
    CHECK_FALSE(grid.at({0, 0}));
    CHECK_THROWS_AS(grid.at({-1, 0}), std::out_of_range);
    CHECK_THROWS_AS(grid.set({3, 0}, true), std::out_of_range);
}

TEST_CASE("BitStream reads MSB-first and reports remaining bits", "[bit-stream]") {
    constexpr std::array<std::uint8_t, 6> bits{1, 0, 1, 1, 0, 0};
    qr::BitStream stream{bits};

    CHECK(stream.read_bits(3) == 0b101);
    CHECK(stream.remaining() == 3);
    CHECK(stream.read_bits(0) == 0);
    CHECK(stream.read_bits(3) == 0b100);
    CHECK(stream.remaining() == 0);
    CHECK_THROWS_AS(stream.read_bits(1), std::out_of_range);
    CHECK_THROWS_AS(qr::BitStream{bits}.read_bits(33), std::invalid_argument);
}

TEST_CASE("FunctionPatternMap marks fixed and version-specific modules", "[patterns]") {
    const qr::FunctionPatternMap version1{1};
    CHECK(version1.is_function_pattern({0, 0}));
    CHECK(version1.is_function_pattern({20, 0}));
    CHECK(version1.is_function_pattern({0, 20}));
    CHECK(version1.is_function_pattern({6, 12}));
    CHECK(version1.is_function_pattern({12, 6}));
    CHECK(version1.is_function_pattern({8, 13}));
    CHECK_FALSE(version1.is_function_pattern({9, 9}));
    CHECK_FALSE(version1.is_function_pattern({-1, 0}));

    const qr::FunctionPatternMap version2{2};
    CHECK(version2.is_function_pattern({18, 18}));
    CHECK(version2.is_function_pattern({16, 16}));
    CHECK(version2.is_function_pattern({20, 20}));

    CHECK_THROWS_AS(qr::FunctionPatternMap{0}, std::invalid_argument);
    CHECK_THROWS_AS(qr::FunctionPatternMap{5}, std::invalid_argument);
}
