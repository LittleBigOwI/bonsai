#include "interface/modals/confirm.hpp"
#include "interface/piechart.hpp"
#include "interface/sidebar.hpp"
#include "config/config.hpp"
#include "core/scanner.hpp"

#include <ftxui/component/screen_interactive.hpp>

#include <thread>

#define DEFAULT_PATH "/home/littlebigowl/Downloads"

int main() {
    using namespace ftxui;

    const Config& config = Config::get();

    ScreenInteractive screen = ScreenInteractive::Fullscreen();
    
    ScanSnapshot snapshot;
    Scanner scanner(DEFAULT_PATH, &snapshot);

    auto sidebar = ui::sidebar(snapshot, config.SIDEBAR_WIDTH, DEFAULT_PATH);    
    auto piechart = ui::piechart(snapshot, DEFAULT_PATH);

    bool modal_shown = false;

    auto show_modal = [&] { modal_shown = true; };
    auto hide_modal = [&] { modal_shown = false; };

    sidebar->setOnDeleteCallback([&](const std::string& path) {
        show_modal();
    });

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
        auto content = hbox({
            leftbox_layout->Render(),
            piechart->Render(),
        });

        return content;
    });

    auto confirm_modal = ui::confirm(
        "Are you sure you want to",
        "remove the selected file / folder?",
        "confirm", 
        "cancel"
    );

    confirm_modal->setOnConfirmCallback([&] {
        auto node = sidebar->getSelectedNode();
        if(!node) return;
        scanner.deleteNode(node);
        hide_modal();
        sidebar->rebuild();
        piechart->rebuild();
    });

    confirm_modal->setOnCloseCallback([&] {
        hide_modal();
    });

    ui |= Modal(confirm_modal, &modal_shown);

    std::thread scan_thread([&] {
        
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
