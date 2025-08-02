#pragma once

#include <string>
#include <vector>
#include <ftxui/component/component.hpp>
#include <ftxui/screen/color.hpp>

namespace ui {

struct RingSlice {
    std::string label;
    double percent = 0.0;
    std::vector<RingSlice> children;
};

struct PieRing {
    std::vector<RingSlice> slices;
};

ftxui::Component RenderPieChart(const PieRing& ring, const std::string& center_label);

}