#pragma once
#include "../core/scanner.hpp"
#include "../utils/colors.hpp"

#include <ftxui/component/component.hpp>

namespace ui {

struct RingSlice {
    std::string label;
    double percent;
    std::vector<RingSlice> children;
};

struct PieRing {
    std::vector<RingSlice> slices;
};

class PieChart : public ftxui::ComponentBase {
public:
    PieChart(PieRing ring, ScanSnapshot& snapshot);

private:
    ftxui::Element OnRender() override;

    PieRing ring_;
    ScanSnapshot& snapshot_;
    int renderCount_ = 0;
};

ftxui::Component MakePieChart(PieRing ring, ScanSnapshot& snapshot);
ftxui::Component BuildPieRingFromSnapshot(ScanSnapshot& snapshot, const std::string& root_dir, int max_rings = 4, double min_percent = 1.0);

}
