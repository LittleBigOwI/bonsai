#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#define SIDEBAR_WIDTH 50

int main() {
    auto sidebar_renderer = ftxui::Renderer([&] {
        return ftxui::vbox({
            ftxui::text("Hello World") | ftxui::bold | ftxui::center,
        }) | ftxui::border | size(ftxui::WIDTH, ftxui::EQUAL, SIDEBAR_WIDTH);
    });

    auto right_pane = ftxui::Renderer([&] {
        return ftxui::vbox({
            ftxui::text("Main Content Area") | ftxui::bold | ftxui::center,
            ftxui::filler(),
        }) | ftxui::border;
    });

    auto app = ftxui::Renderer([&] {
        return ftxui::hbox({
            sidebar_renderer->Render(),
            right_pane->Render() | ftxui::flex,
        });
    });

    ftxui::ScreenInteractive::Fullscreen().Loop(app);
}

