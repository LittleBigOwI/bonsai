#include "widgets/canvas_ext.hpp"
#include "interface/piechart.hpp"
#include "interface/sidebar.hpp"
#include "core/scanner.hpp"

#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include <cmath>
#include <iostream>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <thread>
#include <iomanip>

#define SIDEBAR_WIDTH 50
#define SIDEBAR_PATH "/"

int main() {
    using namespace ftxui;

    auto screen = ScreenInteractive::Fullscreen();

    std::vector<ui::PieRing> rings = {
        {
            {{"home 50%", 50.0}, {"usr 30%", 30.0}, {"bin 20%", 20.0}}
        }
    };
    std::string center_label = "140.29\nGiB";
    auto right_pane = ui::RenderPieChart(rings, center_label);

    auto sidebar = ui::RenderSidebar(SIDEBAR_PATH, SIDEBAR_WIDTH);

    auto app = Container::Horizontal({
        sidebar,
        right_pane,
    });

    screen.Loop(app);

    // Scanner scanner(SIDEBAR_PATH);
    // scanner.scan();

    // const auto& sizes = scanner.getSizes();
    // uintmax_t total = scanner.getTotalSize();
    // uintmax_t files = scanner.getFileCount();

    // std::vector<std::pair<std::string, uintmax_t>> sorted(sizes.begin(), sizes.end());
    // std::sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) {
    //     return b.second < a.second;
    // });

    // std::cout << "Scanned " << files << " files totaling " << (total / (1024 * 1024)) << " MB.\n";

    // for (const auto& [path, size] : sorted) {
    //     double percent = (total > 0) ? (double(size) / total) * 100.0 : 0.0;
    //     if (percent < 0.1) continue;
    //     std::cout << std::setw(8) << (size / (1024 * 1024)) << " MB  "
    //               << std::fixed << std::setprecision(2) << percent << "%  "
    //               << path << "\n";
    // }

    // return 0;

    //TODO: Update ring renderer to support multiple rings. for now it just draws one ring with labels.
}