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
#define SIDEBAR_PATH "/"

void PrintTree(const std::shared_ptr<TreeNode>& node, int depth = 0) {
    if (!node || depth >= 3) return;
    
    std::string indent(depth * 2, ' ');
    std::cout << indent << (node->is_dir ? 
        "[DIR] " : "[FILE] ") << node->name << 
        " | size: " << node->size << 
        " | files: " << node->files << 
        " | folders: " << node->folders << "\n";
        
    for (auto& child : node->children) {
        PrintTree(child, depth + 1);
    }
}

int main() {
    using namespace ftxui;


    ScreenInteractive screen = ScreenInteractive::Fullscreen();
    ScanSnapshot snapshot;

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

    // std::thread scan_thread([&] {
        Scanner scanner(SIDEBAR_PATH, &snapshot);
        // scanner.setCallback([&]() {
        //     std::lock_guard<std::mutex> lock(snapshot.tree_mutex);
        //     screen.PostEvent(Event::Custom);
        // });

        scanner.scan();

    //     screen.PostEvent(Event::Escape);
    // });

    // PrintTree(snapshot.root);

    auto sidebar = ui::sidebar(snapshot, SIDEBAR_WIDTH, SIDEBAR_PATH);
    auto sidebox = Container::Vertical({
        sidebar
    });

    auto sidebox_renderer = Renderer(sidebox, [&]{
        return vbox({
            text("Explorer") | bold | center,
            separator(),
            sidebar->Render() | vscroll_indicator | frame
        }) | border | size(WIDTH, EQUAL, SIDEBAR_WIDTH);
    });

    screen.Loop(sidebox_renderer);
    // scan_thread.join();

    return 0;
}
