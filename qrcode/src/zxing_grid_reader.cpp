#include <qrcode/zxing_grid_reader.h>

#include <BitMatrix.h>
#include <HybridBinarizer.h>
#include <ZXingCpp.h>

#include <array>
#include <memory>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace qr {
std::expected<BitGrid, std::string> ZxingGridReader::read_grid(std::string image_path) const {
    int width{};
    int height{};
    int channels{};

    std::unique_ptr<stbi_uc, void (*)(void*)> buffer{
        stbi_load(image_path.c_str(), &width, &height, &channels, 0),
        stbi_image_free,
    };

    if (!buffer) {
        return std::unexpected{"Failed to load image"};
    }

    constexpr int quiet_zone = 4;
    constexpr int minimum_size = 21;
    constexpr int maximum_size = 33;
    constexpr int base_size = 17;
    constexpr int version_step = 4;

    const int size = width - 2 * quiet_zone;
    if (width != height || size < minimum_size || size > maximum_size ||
        (size - base_size) % version_step != 0) {
        return std::unexpected{"expected an unscaled QR version 1-4 image with a 4-module quiet zone"};
    }

    const auto formats = std::array{
        ZXing::ImageFormat::None,
        ZXing::ImageFormat::Lum,
        ZXing::ImageFormat::LumA,
        ZXing::ImageFormat::RGB,
        ZXing::ImageFormat::RGBA,
    };

    if (channels < 1 || channels >= static_cast<int>(formats.size())) {
        return std::unexpected{"unsupported image channel count"};
    }

    const ZXing::ImageView image{buffer.get(), width, height, formats[channels]};
    const auto cropped = image.cropped(quiet_zone, quiet_zone, size, size);
    const auto bitmap = std::make_unique<ZXing::HybridBinarizer>(cropped)->getBlackMatrix();

    BitGrid grid{size, size};
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            grid.set({x, y}, bitmap->get(x, y));
        }
    }

    return grid;
}

} // namespace qr
