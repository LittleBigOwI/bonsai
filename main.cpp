#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include "ui/sidebar.hpp"

#define SIDEBAR_WIDTH 50
#define SIDEBAR_PATH "/"

int main() {
    using namespace ftxui;

    auto screen = ScreenInteractive::Fullscreen();

    auto right_pane_renderer = Renderer([] {
        return vbox({
            text("Main Content Area") | center,
            filler(),
        }) | border | flex;
    });

    auto sidebar = ui::RenderSidebar(SIDEBAR_PATH, SIDEBAR_WIDTH);

    auto app = Container::Horizontal({
        sidebar,
        right_pane_renderer,
    });

    screen.Loop(app);
}