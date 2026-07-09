#include <qrcode/function_patterns.h>

#include <stdexcept>

namespace qr {

FunctionPatternMap::FunctionPatternMap(int version) : reserved_{21, 21} {
    if (version != 1) {
        throw std::invalid_argument{"FunctionPatternMap currently supports only QR version 1"};
    }

    constexpr int size = 21;

    auto mark = [this](ModuleCoord p) {
        if (reserved_.in_bounds(p)) {
            reserved_.set(p, true);
        }
    };

    auto mark_rect = [&mark](int start_x, int start_y, int width, int height) {
        for (int y = start_y; y < start_y + height; ++y) {
            for (int x = start_x; x < start_x + width; ++x) {
                mark({x, y});
            }
        }
    };

    mark_rect(0, 0, 8, 8);
    mark_rect(size - 8, 0, 8, 8);
    mark_rect(0, size - 8, 8, 8);

    for (int i = 0; i < size; ++i) {
        mark({i, 6});
        mark({6, i});
    }

    for (int i = 0; i <= 5; ++i) {
        mark({i, 8});
        mark({8, i});
    }

    mark({7, 8});
    mark({8, 7});
    mark({8, 8});

    for (int x = size - 8; x < size; ++x) {
        mark({x, 8});
    }

    for (int y = size - 7; y < size; ++y) {
        mark({8, y});
    }

    mark({8, 4 * version + 9});
}

bool FunctionPatternMap::is_function_pattern(ModuleCoord p) const noexcept {
    return reserved_.in_bounds(p) && reserved_.at(p);
}

} // namespace qr
