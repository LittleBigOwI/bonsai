#pragma once
#include <ftxui/screen/color.hpp>
#include <vector>

namespace utils {

struct RGB {
    int r, g, b;
    ftxui::Color toFTXUIColor() const;
};

extern const std::vector<RGB> base_palette;

RGB dim(const RGB& color, float factor);
ftxui::Color getContrastColor(const RGB& background);

}