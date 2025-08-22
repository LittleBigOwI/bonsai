#pragma once

#include <ftxui/screen/color.hpp>

#include <algorithm>
#include <vector>

namespace utils {
    struct RGB {
        int r, g, b;

        RGB(int r_ = 0, int g_ = 0, int b_ = 0) : r(r_), g(g_), b(b_) {}
        RGB(unsigned long color) {
            r = (color >> 16) & 0xFF;
            g = (color >> 8) & 0xFF;
            b = color & 0xFF;
        }

        ftxui::Color toFTXUIColor() const;
    };

    extern const std::vector<RGB> base_palette;

    RGB dim(const RGB& color, float factor);
    ftxui::Color getContrastColor(const RGB& background);

    const RGB white = {255, 255, 255};
}
