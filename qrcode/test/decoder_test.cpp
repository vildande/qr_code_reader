#include <qrcode/decoder.h>
#include <qrcode/function_patterns.h>
#include <qrcode/zxing_grid_reader.h>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

qr::BitGrid load_grid(const std::string& path) {
    auto grid = qr::ZxingGridReader{}.read_grid(path);
    if (!grid) {
        throw std::runtime_error{grid.error()};
    }
    return std::move(*grid);
}

std::string decode_file(const std::string& path) {
    auto grid = load_grid(path);
    const int version = (grid.width() - 17) / 4;
    return qr::decode_message(grid, qr::FunctionPatternMap{version});
}

std::vector<qr::ModuleCoord> version1_data_positions(
    const qr::FunctionPatternMap& patterns
) {
    std::vector<qr::ModuleCoord> positions;
    bool upward = true;

    for (int right_x = 20; right_x > 0; right_x -= 2) {
        if (right_x == 6) {
            --right_x;
        }
        for (int step = 0; step < 21; ++step) {
            const int y = upward ? 20 - step : step;
            for (int offset = 0; offset < 2; ++offset) {
                const qr::ModuleCoord position{right_x - offset, y};
                if (!patterns.is_function_pattern(position)) {
                    positions.push_back(position);
                }
            }
        }
        upward = !upward;
    }
    return positions;
}

} // namespace

TEST_CASE("Decoder reads supplied and generated QR fixtures", "[decoder]") {
    struct Fixture {
        std::string path;
        std::string expected;
    };
    const std::array fixtures{
        Fixture{QR_SAMPLE_DATA_DIR "/qr01.png", "12345"},
        Fixture{QR_SAMPLE_DATA_DIR "/qr02.png", "314159"},
        Fixture{QR_SAMPLE_DATA_DIR "/qr03.png", "Hello World"},
        Fixture{QR_SAMPLE_DATA_DIR "/qr04.png", "Intro. to C++"},
        Fixture{QR_SAMPLE_DATA_DIR "/qr05.png", "1 + 2 is 3"},
        Fixture{QR_TEST_DATA_DIR "/version2.png", "VERSION 2 MASK 3"},
        Fixture{QR_TEST_DATA_DIR "/version3.png", "VERSION 3 LEVEL H"},
        Fixture{QR_TEST_DATA_DIR "/version4.png", "VERSION 4 LEVEL Q"},
        Fixture{QR_TEST_DATA_DIR "/kanji.png", "点茗テ"},
        Fixture{
            QR_TEST_DATA_DIR "/mixed.png",
            "1234567890点茗ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        },
    };

    for (const auto& fixture : fixtures) {
        CAPTURE(fixture.path);
        CHECK(decode_file(fixture.path) == fixture.expected);
    }
}

TEST_CASE("Version 1 Reed-Solomon corrects three damaged codewords", "[decoder][error]") {
    auto grid = load_grid(QR_SAMPLE_DATA_DIR "/qr01.png");
    const qr::FunctionPatternMap patterns{1};
    const auto positions = version1_data_positions(patterns);
    REQUIRE(positions.size() == 208);

    constexpr std::array<std::size_t, 3> damaged_codewords{0, 25, 1};
    for (const auto codeword : damaged_codewords) {
        const auto position = positions[codeword * 8];
        grid.set(position, !grid.at(position));
    }

    CHECK(qr::decode_message(grid, patterns) == "12345");
}

TEST_CASE("Format BCH recovers three damaged bits in both copies", "[decoder][format]") {
    auto grid = load_grid(QR_SAMPLE_DATA_DIR "/qr01.png");
    constexpr std::array first_copy{
        qr::ModuleCoord{0, 8}, qr::ModuleCoord{1, 8}, qr::ModuleCoord{2, 8},
    };
    constexpr std::array second_copy{
        qr::ModuleCoord{8, 20}, qr::ModuleCoord{8, 19}, qr::ModuleCoord{8, 18},
    };

    for (std::size_t index = 0; index < first_copy.size(); ++index) {
        grid.set(first_copy[index], !grid.at(first_copy[index]));
        grid.set(second_copy[index], !grid.at(second_copy[index]));
    }

    CHECK(qr::decode_message(grid, qr::FunctionPatternMap{1}) == "12345");
}

TEST_CASE("Grid reader reports invalid image paths", "[image]") {
    const auto result = qr::ZxingGridReader{}.read_grid("missing-qr-image.png");
    REQUIRE_FALSE(result.has_value());
    CHECK(result.error() == "Failed to load image");
}

TEST_CASE("Decoder rejects a grid without valid format information", "[decoder][invalid]") {
    CHECK_THROWS_AS(
        qr::decode_message(qr::BitGrid{21, 21}, qr::FunctionPatternMap{1}),
        std::invalid_argument
    );
}
