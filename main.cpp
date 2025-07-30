#include "widgets/canvas_ext.hpp"
#include "ui/sidebar.hpp"

#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#define SIDEBAR_WIDTH 50
#define SIDEBAR_PATH "/"

int main() {
    using namespace ftxui;

    // auto screen = ScreenInteractive::Fullscreen();

    // auto right_pane_renderer = Renderer([] {
    //     return vbox({
    //         text("Main Content Area") | center,
    //         filler(),
    //     }) | border | flex;
    // });

    // auto sidebar = ui::RenderSidebar(SIDEBAR_PATH, SIDEBAR_WIDTH);

    // auto app = Container::Horizontal({
    //     sidebar,
    //     right_pane_renderer,
    // });

    // screen.Loop(app);

    int width = 400;
    int height = 400;
    int cx = 25;
    int cy = 25;
    int radius = 30;

    auto c = BonsaiCanvas(width, height);
    c.DrawAngledPointEllipse(200, 200, 30, 30, 160);

    auto document = canvas(&c) | border;
    auto screen = Screen::Create(Dimension::Fit(document));
    Render(screen, document);
    screen.Print();

    return 0;
}