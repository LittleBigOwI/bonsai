#pragma once

#include <string>
#include <vector>
#include <ftxui/component/component.hpp>
#include <ftxui/screen/color.hpp>

namespace ui {

struct RingSlice {
    std::string label;
    double percent = 0.0;
};

struct PieRing {
    std::vector<RingSlice> slices;
};

ftxui::Component RenderPieChart(const std::vector<PieRing>& slices, const std::string& center_label);

}