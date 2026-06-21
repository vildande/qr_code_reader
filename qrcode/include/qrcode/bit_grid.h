#pragma once

#include <cstdint>
#include <vector>

namespace qr {

    struct ModuleCoord {
        int x{};
        int y{};
    };

    class BitGrid {
    public:
        BitGrid(int width, int height);
        [[nodiscard]] int width() const noexcept;
        [[nodiscard]] int height() const noexcept;
        [[nodiscard]] bool in_bounds(ModuleCoord p) const noexcept;

        [[nodiscard]] bool at(ModuleCoord p) const;

        void set(ModuleCoord p, bool value);
    private:
        int width_ {};
        int height_ {};
        std::vector<uint8_t> modules_;
    };

} // namespace qr