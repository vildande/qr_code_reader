#pragma once

#include <cstdint>
#include <vector>

/// QR decoding data structures and algorithms.
namespace qr {

/// Integer coordinates of one QR module, with 'x' as column and `y` as row.
struct ModuleCoord {
    int x{}; ///< Column from the left.
    int y{}; ///< Row from the top.
};

/// A rectangular, bounds-checked grid of black and white QR modules.
class BitGrid {
public:
    /// Creates a white grid with the requested dimensions.
    BitGrid(int width, int height);

    /// Returns the number of columns.
    [[nodiscard]] int width() const noexcept;

    /// Returns the number of rows.
    [[nodiscard]] int height() const noexcept;

    /// Reports whether a coordinate lies inside the grid.
    [[nodiscard]] bool in_bounds(ModuleCoord p) const noexcept;

    /// Returns one module.
    /// @throws std::out_of_range if `p` is outside the grid.
    [[nodiscard]] bool at(ModuleCoord p) const;

    /// Changes one module.
    /// @throws std::out_of_range if `p` is outside the grid.
    void set(ModuleCoord p, bool value);

private:
    int width_{};
    int height_{};
    std::vector<std::uint8_t> modules_;
};

} // namespace qr
