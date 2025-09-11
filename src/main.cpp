#include "interface/piechart.hpp"
#include "interface/sidebar.hpp"
#include "config/config.hpp"
#include "core/scanner.hpp"

#include <ftxui/component/screen_interactive.hpp>

#include <iostream>
#include <thread>

#define DEFAULT_PATH "/"

int main() {
    using namespace ftxui;

    const Config& config = Config::get();

    ScreenInteractive screen = ScreenInteractive::Fullscreen();
    ScanSnapshot snapshot;

    auto sidebar = ui::sidebar(snapshot, config.SIDEBAR_WIDTH, DEFAULT_PATH);    
    auto piechart = ui::piechart(snapshot, DEFAULT_PATH);

    sidebar->setOnChangeCallback([&](const std::string& path) {
        piechart->setSelected(path);
    });

    sidebar->setOnEnterCallback([&](const std::string& path) {
        piechart->setPath(path);
    });

    auto sidebar_panel = Renderer(sidebar, [&] {
        return vbox({
            text("Explorer") | bold | center | color(Color::White),
            separator(),
            sidebar->Render() | vscroll_indicator | frame
        }) | size(WIDTH, EQUAL, config.SIDEBAR_WIDTH) | border | color(Color::GrayDark);
    });
    
    auto leftbox_layout = Container::Horizontal({
        sidebar_panel,
    });
    
    auto ui = Renderer(leftbox_layout, [&] {
        return hbox({
            leftbox_layout->Render(),
            piechart->Render(),
        });
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
