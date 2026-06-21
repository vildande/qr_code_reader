#include <qrcode/bit_grid.h>

#include <stdexcept>

namespace qr {
    BitGrid::BitGrid(int width, int height)
    : width_(width), height_(height), modules_(width * height) {}

    int BitGrid::width() const noexcept { return width_; }
    int BitGrid::height() const noexcept { return height_; }

    bool BitGrid::in_bounds(ModuleCoord p) const noexcept {
        return p.x >= 0 && p.x < width_ && p.y >= 0 && p.y < height_;
    }

    bool BitGrid::at(ModuleCoord p) const {
        if (!in_bounds(p)) {
            throw std::out_of_range{"BitGrid coordinate out of range"};
        }
        return modules_[p.y * width_ + p.x] != 0;
    }

    void BitGrid::set(ModuleCoord p, bool value) {
        if (!in_bounds(p)) {
            throw std::out_of_range{"BitGrid coordinate out of range"};
        }
        modules_[p.y * width_ + p.x] = static_cast<std::uint8_t>(value);
    }

} // namespace qr
