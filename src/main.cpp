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

    ui::PieRing ring = {
        {
            {
                "home 50%", 50.0, {
                    {
                        "pictures 30%", 30.0, {
                            {"phone 30%", 30.0}
                        }
                    },
                    {"videos 10%", 10.0}
                }
            },
            {
                "usr 30%", 30.0, {
                    {"lib 15%", 15.0},
                }
            },
            {
                "bin 20%", 20.0, {
                    {
                        "librewolf 10%", 10.0, {
                            {"internal 10%", 10.0}
                        }
                    },
                }
            }
        }
    };
    std::string center_label = "140.29\nGiB";
    auto right_pane = ui::RenderPieChart(ring, center_label);

    auto sidebar = ui::RenderSidebar(SIDEBAR_PATH, SIDEBAR_WIDTH);

    auto app = Container::Horizontal({
        sidebar,
        right_pane,
    });

    screen.Loop(app);
}