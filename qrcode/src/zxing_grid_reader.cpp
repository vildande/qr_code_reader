#include <qrcode/zxing_grid_reader.h>

#include <BitMatrix.h>
// #include <HybridBinarizer.h>
// #include <ZXingCpp.h>
///#include <qrcode/QRBitMatrixParser.h>

/// #include <bitset>
/// #include <cstdint>



#include <HybridBinarizer.h>
#include <ZXingCpp.h>

#include <array>
#include <memory>


#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


namespace qr {
    std::expected<BitGrid, std::string> ZxingGridReader::read_version1_grid(std::string image_path) const {
        int width{};
        int height{};
        int channels{};
        
        std::string path{image_path};

        // buffer
        std::unique_ptr<stbi_uc, void (*)(void*)> buffer (
            stbi_load(path.c_str(), &width, &height, &channels, 0), stbi_image_free
        );

        if (!buffer) {
            return std::unexpected{"Failed to load image"};
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

        ZXing::ImageView image{
            buffer.get(),
            width,
            height,
            formats[channels],
        };

        // Crop size is always 4 in the test cases.
        // Change the version if you are not reading Version 1.
        int version = 1;
        int crop_offset = 4;
        auto size = 17 + 4 * version;
        auto cropped = image.cropped(crop_offset, crop_offset, size, size);

        // bitmap is a matrix of bool (true: black, false: white).
        auto bitmap = std::make_unique<ZXing::HybridBinarizer>(cropped)->getBlackMatrix();

        BitGrid grid{size, size};
        
        for (int y = 0; y < size; ++y) {
            for (int x = 0; x < size; ++x) {
                grid.set({x, y}, bitmap->get(x, y));
            }
        }

        return grid;
    }
} // namespace qr
