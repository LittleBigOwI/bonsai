#include "interface/piechart.hpp"
#include "interface/sidebar.hpp"
#include "core/scanner.hpp"

#include <ftxui/component/screen_interactive.hpp>

#define DEFAULT_PATH "/home/littlebigowl/Documents"
#define SIDEBAR_WIDTH 50

int main() {
    using namespace ftxui;


    ScreenInteractive screen = ScreenInteractive::Fullscreen();
    ScanSnapshot snapshot;

    // std::thread scan_thread([&] {
        Scanner scanner(DEFAULT_PATH, &snapshot);
        // scanner.setCallback([&]() {
        //     std::lock_guard<std::mutex> lock(snapshot.tree_mutex);
        //     screen.PostEvent(Event::Custom);
        // });

        scanner.scan();

    //     screen.PostEvent(Event::Escape);
    // });

    auto sidebar = ui::sidebar(snapshot, SIDEBAR_WIDTH, DEFAULT_PATH);    
    auto piechart = ui::piechart(snapshot, DEFAULT_PATH);

    sidebar->setOnEnterCallback([&](const std::string& path) {
        piechart->setPath(path);
    });

    auto sidebar_panel = Renderer(sidebar, [&] {
        return vbox({
            text("Explorer") | bold | center,
            separator(),
            sidebar->Render() | vscroll_indicator | frame
        }) | size(WIDTH, EQUAL, SIDEBAR_WIDTH);
    });
    
    // This is so that the sidebar is focusable
    auto leftbox_layout = Container::Horizontal({
        sidebar_panel,
    });
    
    auto ui = Renderer(leftbox_layout, [&] {
        return hbox({
            leftbox_layout->Render(),
            separator(),
            piechart->Render(),
        }) | border;
    });

    screen.Loop(ui);
    // scan_thread.join();
    return 0;
}
