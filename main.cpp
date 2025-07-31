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

    auto c = BonsaiCanvas(width, height);
    c.DrawAngledPointEllipseRing(150, 150, 60, 60, 30, 360, Color::Red);
    c.DrawAngledPointEllipseRing(150, 150, 60, 60, 30, 180, Color::Blue);
    c.DrawAngledPointEllipseRing(150, 150, 60, 60, 30, 60, Color::Green);

    auto document = canvas(&c) | border;
    auto screen = Screen::Create(Dimension::Fit(document));
    Render(screen, document);
    screen.Print();

    return 0;
}