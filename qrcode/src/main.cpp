#include <qrcode/decoder.h>
#include <qrcode/function_patterns.h>
#include <qrcode/zxing_grid_reader.h>

#include <exception>
#include <iostream>
#include <print>
#include <string>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "The correct way to to run is: qrcode <image>\n";
        return 1;
    }

    const qr::ZxingGridReader reader;
    auto grid = reader.read_grid(std::string{argv[1]});
    if (!grid) {
        std::cerr << "Error reading image: " << grid.error() << '\n';
        return 1;
    }

    try {
        const int version = (grid->width() - 17) / 4;
        const qr::FunctionPatternMap function_patterns{version};
        std::string message = qr::decode_message(*grid, function_patterns);
        std::println("{}", message);
    } catch (const std::exception& error) {
        std::cerr << "Decode error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
