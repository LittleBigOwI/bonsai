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
#define DEFAULT_PATH "/home/littlebigowl/Pictures"

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

    // auto ui = Container::Horizontal({ sidebar });

    // std::thread scan_thread([&] {
        Scanner scanner(DEFAULT_PATH, &snapshot);
        // scanner.setCallback([&]() {
        //     std::lock_guard<std::mutex> lock(snapshot.tree_mutex);
        //     screen.PostEvent(Event::Custom);
        // });

        scanner.scan();

    //     screen.PostEvent(Event::Escape);
    // });

    // PrintTree(snapshot.root);

    auto sidebar = ui::sidebar(snapshot, SIDEBAR_WIDTH, DEFAULT_PATH);    
    auto chart = ui::piechart(snapshot, DEFAULT_PATH);

    auto leftbox = Renderer(sidebar, [&] {
        return vbox({
            text("Explorer") | bold | center,
            separator(),
            sidebar->Render() | vscroll_indicator | frame
        }) | size(WIDTH, EQUAL, SIDEBAR_WIDTH);
    });

    auto mainbox = Renderer([] {
        return filler();
    });

    auto ui = Container::Horizontal({
        leftbox,
        mainbox,
    });

    auto ui_renderer = Renderer(ui, [&] {
        return hbox({
            leftbox->Render(),
            separator(),
            chart->Render(),
        }) | border;
    });

    // screen.Loop(ui_renderer);
    // scan_thread.join();
    return 0;
}
