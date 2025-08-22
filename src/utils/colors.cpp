#include "colors.hpp"

#include "../config/config.hpp"

#include <algorithm>

namespace utils {    
    const std::vector<RGB> base_palette = []() -> std::vector<RGB> {
        std::vector<RGB> v;
        for (auto& arr : Config::get().CHART_COLORS) {
            v.emplace_back(arr[0], arr[1], arr[2]);
        }
        return v;
    }();
    
    ftxui::Color RGB::toFTXUIColor() const {
        return ftxui::Color::RGB(std::clamp(r, 0, 255), std::clamp(g, 0, 255), std::clamp(b, 0, 255));
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
