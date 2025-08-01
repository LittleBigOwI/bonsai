#include "widgets/canvas_ext.hpp"
#include "interface/sidebar.hpp"

#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include <cmath>

#define SIDEBAR_WIDTH 50
#define SIDEBAR_PATH "/"

int main() {
    using namespace ftxui;

    auto screen = ScreenInteractive::Fullscreen();

    auto right_pane = Renderer([] {
        // App preview
        return canvas([](Canvas& c) {
            int width = c.width();
            int height = c.height();

            int cx = width / 2;
            int cy = height / 2;

            int radius = std::min(width, height) / 6;

            BonsaiCanvas bonsai(c);
            bonsai.DrawAngledBlockCircleRing(cx, cy, radius*2, (radius*1.50), 360, Color::Red3);
            bonsai.DrawAngledBlockCircleRing(cx, cy, radius*2, (radius*1.50), 180, Color::Blue3);
            bonsai.DrawAngledBlockCircleRing(cx, cy, radius*2, (radius*1.50), 60, Color::Green3);

            bonsai.DrawAngledBlockCircleRing(cx, cy, radius*1.50, radius, 360, Color::Red);
            bonsai.DrawAngledBlockCircleRing(cx, cy, radius*1.50, radius, 180, Color::Blue1);
            bonsai.DrawAngledBlockCircleRing(cx, cy, radius*1.50, radius, 60, Color::Green);

            bonsai.DrawAngledBlockCircleRing(cx, cy, radius, radius/2, 360, Color::Red1);
            bonsai.DrawAngledBlockCircleRing(cx, cy, radius, radius/2, 180, Color::Blue);
            bonsai.DrawAngledBlockCircleRing(cx, cy, radius, radius/2, 60, Color::Green1);

            int label_radius = radius - (radius / 4);

            c.DrawText(cx - 5, cy - 2, "140.29");
            c.DrawText(cx - 2, cy + 2, "GiB");
            c.DrawText(
                cx - 9,
                cy - radius + (radius / 4),
                "/home 50%",
                [](ftxui::Pixel &p) {
                    p.background_color = ftxui::Color::Red1;
                    p.foreground_color = ftxui::Color::White;
                }
            );

            {
                std::string text = "/usr 30%";
                int text_len = static_cast<int>(text.size());
                double angle_deg = 240.0;
                double angle_rad = angle_deg * M_PI / 180.0;
                angle_rad = 2 * M_PI - angle_rad;

                int x = static_cast<int>(cx + std::cos(angle_rad) * label_radius) - (text_len / 2);
                int y = static_cast<int>(cy + std::sin(angle_rad) * label_radius);

                c.DrawText(x, y, text, [](ftxui::Pixel &p) {
                    p.background_color = ftxui::Color::Blue;
                    p.foreground_color = ftxui::Color::White;
                });
            }

            {
                std::string text = "/bin 20%";
                int text_len = static_cast<int>(text.size()) + 2;
                double angle_deg = 330.0;
                double angle_rad = angle_deg * M_PI / 180.0;
                angle_rad = 2 * M_PI - angle_rad;

                int x = static_cast<int>(cx + std::cos(angle_rad) * label_radius) - (text_len / 2);
                int y = static_cast<int>(cy + std::sin(angle_rad) * label_radius);

                c.DrawText(x, y, text, [](ftxui::Pixel &p) {
                    p.background_color = ftxui::Color::Green1;
                    p.foreground_color = ftxui::Color::Black;
                });
            }

        }) | flex | border;
    });

    auto sidebar = ui::RenderSidebar(SIDEBAR_PATH, SIDEBAR_WIDTH);

    auto app = Container::Horizontal({
        sidebar,
        right_pane,
    });

    screen.Loop(app);
}