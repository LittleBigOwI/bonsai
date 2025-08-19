#pragma once

#include "../widgets/canvas_ext.hpp"
#include "../core/scanner.hpp"
#include "../utils/colors.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include <algorithm>
#include <cmath>

#define CHART_MAX_SIZE_THRESHOLD_PERCENTAGE 10
#define CHART_MAX_GENERATIONS 4

namespace ui {
    using namespace ftxui;

    class PiechartComponent : public ComponentBase {
    public:
        PiechartComponent(ScanSnapshot& snapshot, const std::string& path): snapshot_(snapshot), path_(path) {
            // build();
            // setup();
        }
    
    private:
        std::string path_;
        ScanSnapshot& snapshot_;
    };

     inline Component piechart(ScanSnapshot& snapshot, const std::string path) {
        return Make<PiechartComponent>(snapshot, path);
    }
}