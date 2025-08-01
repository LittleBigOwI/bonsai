#include "piechart.hpp"
#include "../widgets/canvas_ext.hpp"
#include "../utils/colors.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <cmath>
#include <algorithm>

using namespace ftxui;

namespace ui {

Component RenderPieChart(const std::vector<PieRing>& rings, const std::string& center_label) {
    return Renderer([rings, center_label] {
        return canvas([rings, center_label](Canvas& c) {
            int width = c.width();
            int height = c.height();

            int cx = width / 2;
            int cy = height / 2;

            int radius = std::min(width, height) / 6;

            auto sorted_rings = rings;

            for (auto& ring : sorted_rings) {
                std::sort(ring.slices.begin(), ring.slices.end(), [](const RingSlice& a, const RingSlice& b) {
                    return a.percent > b.percent;
                });
            }
            
            BonsaiCanvas bonsai(c);

            size_t split = center_label.find("\n");
            if (split != std::string::npos) {
                std::string line1 = center_label.substr(0, split);
                std::string line2 = center_label.substr(split + 1);
                c.DrawText((cx - .5) - int(line1.size()) / 2, cy - 2, line1);
                c.DrawText((cx - .5) - int(line2.size()) / 2, cy + 2, line2);
            } else {
                c.DrawText(cx - int(center_label.size()) / 2, cy, center_label);
            }

            for (int i = sorted_rings.size() - 1; i >= 0; --i) {
                const auto& ring = sorted_rings[i];
                
                for (size_t j = 0; j < ring.slices.size(); ++j) {
                    double angle_offset = 0.0;
                    for (size_t k = j + 1; k < ring.slices.size(); ++k) {
                        angle_offset += ring.slices[k].percent / 100.0 * 360;
                    }

                    const auto& slice = ring.slices[j];
                    double angle = slice.percent / 100.0 * 360.0;
                    const utils::RGB color = utils::base_palette[j];
                    
                    int outer_r = i == 0 ? radius : 1 + (.5 * i);
                    int inner_r = i == 0 ? radius/2 : (i == 1 ? radius : .5 + (.5 * i));

                    bonsai.DrawAngledBlockCircleRing(cx, cy, outer_r, inner_r, angle_offset + angle, color.toFTXUIColor());
                    
                    double label_radius = inner_r + ((outer_r - inner_r) / 2);

                    int x = static_cast<int>(cx + std::cos((angle_offset + (angle / 2.0)) * M_PI / 180.0) * label_radius) - (slice.label.size() / 2) - 1;
                    int y = static_cast<int>(cy + std::sin((angle_offset + (angle / 2.0)) * M_PI / 180.0) * label_radius + .5);

                    c.DrawText(x, y, slice.label, [color](Pixel &p) {
                        p.background_color = color.toFTXUIColor();
                        p.foreground_color = Color::Black;
                    });
                }
            }
        }) | flex | border;
    });
}

}