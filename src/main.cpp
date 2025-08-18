#include "widgets/canvas_ext.hpp"
#include "interface/piechart.hpp"
#include "interface/sidebar.hpp"
#include "utils/dataformat.hpp"
#include "core/scanner.hpp"

#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include <iostream>

#define SIDEBAR_WIDTH 50
#define SIDEBAR_PATH "/home/littlebigowl/Documents/Code/Cpp/bonsai/src"

int main() {
    using namespace ftxui;


    ScreenInteractive screen = ScreenInteractive::Fullscreen();
    ScanSnapshot snapshot;

    // auto sidebar = ui::RenderSidebar(SIDEBAR_PATH, SIDEBAR_WIDTH, snapshot);


    // ui::PieRing ring;
    // ring.slices = {
    //     ui::RingSlice{ "A", 40.0, {} },
    //     ui::RingSlice{ "B", 30.0, {} },
    //     ui::RingSlice{ "C", 20.0, {} },
    //     ui::RingSlice{ "D", 10.0, {} },
    // };

    // ring.slices[0].children = {
    //     ui::RingSlice{ "A1", 25.0, {} },
    //     ui::RingSlice{ "A2", 15.0, {} },
    // };

    // // auto piechart = ui::BuildPieRingFromSnapshot(snapshot, "/");
    // auto ui = Container::Horizontal({ sidebar });

    std::thread scan_thread([&] {
        Scanner scanner(SIDEBAR_PATH, &snapshot);
        scanner.setCallback([&]() {
            std::lock_guard<std::mutex> lock(snapshot.tree_mutex);
            // screen.PostEvent(Event::Custom);
        });

        scanner.scan();

        // screen.PostEvent(Event::Custom);
    });


    // screen.Loop(ui);
    scan_thread.join();
    return 0;
}
