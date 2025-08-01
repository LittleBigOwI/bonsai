#include "colors.hpp"
#include <cmath>
#include <algorithm>

namespace utils {

const std::vector<RGB> base_palette = {
    {255, 99, 71},     // Tomato Red
    {30, 144, 255},    // Dodger Blue
    {60, 179, 113},    // Medium Sea Green
    {255, 165, 0},     // Orange
    {186, 85, 211},    // Medium Orchid
    {255, 215, 0},     // Gold
    {70, 130, 180},    // Steel Blue
    {255, 105, 180},   // Hot Pink
    {154, 205, 50},    // Yellow Green
    {255, 140, 0}      // Dark Orange
};

ftxui::Color RGB::toFTXUIColor() const {
    return ftxui::Color::RGB(std::clamp(r, 0, 255),
                             std::clamp(g, 0, 255),
                             std::clamp(b, 0, 255));
}

RGB dim(const RGB& color, float factor) {
    return {
        static_cast<int>(color.r * factor),
        static_cast<int>(color.g * factor),
        static_cast<int>(color.b * factor)
    };
}

ftxui::Color getContrastColor(const RGB& background) {
    double luminance = 0.299 * background.r + 0.587 * background.g + 0.114 * background.b;
    return luminance > 186 ? ftxui::Color::Black : ftxui::Color::White;
}

}
