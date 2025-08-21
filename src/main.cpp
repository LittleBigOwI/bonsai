#include "interface/piechart.hpp"
#include "interface/sidebar.hpp"
#include "core/scanner.hpp"

#include <ftxui/component/screen_interactive.hpp>
#include <thread>

#define DEFAULT_PATH "/"
#define SIDEBAR_WIDTH 50

int main() {
    using namespace ftxui;

    ScreenInteractive screen = ScreenInteractive::Fullscreen();
    ScanSnapshot snapshot;

    auto sidebar = ui::sidebar(snapshot, SIDEBAR_WIDTH, DEFAULT_PATH);    
    auto piechart = ui::piechart(snapshot, DEFAULT_PATH);

    sidebar->setOnChangeCallback([&](const std::string& path) {
        piechart->setSelected(path);
    });

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

    std::thread scan_thread([&] {
        Scanner scanner(DEFAULT_PATH, &snapshot);
        
        scanner.setCallback([&]() {
            // sidebar->rebuild();
            // piechart->rebuild();

            screen.PostEvent(Event::Custom);
        });

        scanner.scan();

        sidebar->rebuild();
        piechart->rebuild();
        screen.PostEvent(Event::Custom);
    });
    
    scan_thread.detach();
    screen.Loop(ui);

    return 0;
}
