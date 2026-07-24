#pragma once

#include <qrcode/bit_grid.h>

#include <expected>
#include <string>

namespace qr {

/// Converts a QR bitmap into the module grid.
class ZxingGridReader {
public:
    /// Loads a Version 1-4 image with a four-module quiet zone.
    /// @return A grid on success, otherwise a short user-facing error.
    [[nodiscard]] std::expected<BitGrid, std::string> read_grid(std::string image_path) const;
};

} // namespace qr
