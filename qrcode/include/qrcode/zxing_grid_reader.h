#pragma once

#include <qrcode/bit_grid.h>

#include <expected>
#include <string>

namespace qr {

class ZxingGridReader {
public:
    [[nodiscard]] std::expected<BitGrid, std::string> read_version1_grid(std::string image_path) const;
};

} // namespace qr