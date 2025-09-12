#include "interface/piechart.hpp"
#include "interface/sidebar.hpp"
#include "config/config.hpp"
#include "core/scanner.hpp"

#include <ftxui/component/screen_interactive.hpp>

#include <iostream>
#include <thread>

#define DEFAULT_PATH "/home/littlebigowl/Documents/Code/Cpp/bonsai"

// ftxui::Component ModalComponent(std::string subtitle, std::function<void()> do_nothing, std::function<void()> hide_modal) {
//     auto component = ftxui::Container::Vertical({
//         ftxui::Button("Do nothing", do_nothing, ftxui::ButtonOption::Animated()),
//         ftxui::Button("Quit modal", hide_modal, ftxui::ButtonOption::Animated()),
//     });
//     component |= ftxui::Renderer([&](ftxui::Element inner) {
//         return ftxui::vbox({
//             ftxui::text("Modal component"),
//             ftxui::text(subtitle),
//             ftxui::separator(),
//             inner,
//         })
//         | ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, 30)
//         | ftxui::border;
//     });
//     return component;
// }

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
        auto content = hbox({
            text(sidebar->getPath()),
            leftbox_layout->Render(),
            piechart->Render(),
        });

        return content;
    });

    // bool modal_shown = true;

    // auto show_modal = [&] { modal_shown = true; };
    // auto hide_modal = [&] { modal_shown = false; };

    // auto modal_component = ModalComponent(sidebar->getPath(), [&] {}, hide_modal);

    // ui |= Modal(modal_component, &modal_shown);

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
