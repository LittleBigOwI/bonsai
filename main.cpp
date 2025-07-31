#include "widgets/canvas_ext.hpp"
#include "ui/sidebar.hpp"

#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#define SIDEBAR_WIDTH 50
#define SIDEBAR_PATH "/"

int main() {
    using namespace ftxui;

    auto screen = ScreenInteractive::Fullscreen();

    auto right_pane = Renderer([] {
        return canvas([](Canvas& c) {
            int width = c.width();
            int height = c.height();

            int cx = width / 2;
            int cy = height / 2;

            int radius = std::min(width, height) / 3;

            BonsaiCanvas bonsai(c);
            bonsai.DrawAngledBlockCircleRing(cx, cy, radius, radius/2, 360, Color::Red);
            bonsai.DrawAngledBlockCircleRing(cx, cy, radius, radius/2, 180, Color::Blue);
            bonsai.DrawAngledBlockCircleRing(cx, cy, radius, radius/2, 60, Color::Green);
        }) | flex | border;
    });

    auto sidebar = ui::RenderSidebar(SIDEBAR_PATH, SIDEBAR_WIDTH);

    auto app = Container::Horizontal({
        sidebar,
        right_pane,
    });

    screen.Loop(app);
}