#include "interface/modals/confirm.hpp"
#include "interface/modals/error.hpp"

#include "interface/piechart.hpp"
#include "interface/sidebar.hpp"
#include "config/config.hpp"
#include "core/scanner.hpp"

#include <ftxui/component/screen_interactive.hpp>

#include <thread>

#define DEFAULT_PATH "/"

int main() {
    using namespace ftxui;

    const Config& config = Config::get();

    ScreenInteractive screen = ScreenInteractive::Fullscreen();
    
    ScanSnapshot snapshot;
    Scanner scanner(DEFAULT_PATH, &snapshot);

    auto sidebar = ui::sidebar(snapshot, config.SIDEBAR_WIDTH, DEFAULT_PATH);    
    auto piechart = ui::piechart(snapshot, DEFAULT_PATH);

    bool confirm_modal_shown = false;
    auto show_confirm_modal = [&] { confirm_modal_shown = true; };
    auto hide_confirm_modal = [&] { confirm_modal_shown = false; };

    bool error_modal_shown = false;
    auto show_error_modal = [&] { error_modal_shown = true; };
    auto hide_error_modal = [&] { error_modal_shown = false; };

    sidebar->setOnDeleteCallback([&](const std::string& path) {
        show_confirm_modal();
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

    auto error_modal = ui::error(
        "Error while deleting file / folder.",
        "close"
    );

    confirm_modal->setOnConfirmCallback([&] {
        auto node = sidebar->getSelectedNode();
        if(!node) return;
        
        int status = scanner.deleteNode(node);

        hide_confirm_modal();
        if(status == 0) {
            sidebar->rebuild();
            piechart->rebuild();
        } else {
            error_modal->setStatusCode(status);
            show_error_modal();   
        }
    });

    error_modal->setOnCloseCallback([&] {
        hide_error_modal();
    });

    confirm_modal->setOnCloseCallback([&] {
        hide_confirm_modal();
    });

    ui |= Modal(error_modal, &error_modal_shown);
    ui |= Modal(confirm_modal, &confirm_modal_shown);

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
