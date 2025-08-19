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

    screen.Loop(ui_renderer);
    // scan_thread.join();
    return 0;
}
