#include "piechart.hpp"
#include "../widgets/canvas_ext.hpp"
#include "../utils/colors.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <cmath>
#include <algorithm>

using namespace ftxui;

namespace {

void RenderSliceRecursive(Canvas& c, BonsaiCanvas& bonsai, const ui::RingSlice& slice, double angle_offset, int depth, int cx, int cy, int base_radius, const utils::RGB& color) {
    double angle = slice.percent / 100.0 * 360.0;

    int outer_r = base_radius * (1 + (0.5 * depth));
    int inner_r = depth == 0 ? base_radius / 2 : base_radius * (0.5 + (0.5 * depth));

    bonsai.DrawAngledBlockCircleRingOffset(cx, cy, outer_r, inner_r, angle_offset + .5, angle, color.toFTXUIColor());

    double label_radius = inner_r + ((outer_r - inner_r) / 2);
    double angle_mid = angle_offset + angle / 2.0;

    int x = static_cast<int>(cx + std::cos(angle_mid * M_PI / 180.0) * label_radius) - (slice.label.size() / 2) - 1;
    int y = static_cast<int>(cy + std::sin(angle_mid * M_PI / 180.0) * label_radius + 0.5);

    c.DrawText(x, y, slice.label, [color](Pixel &p) {
        p.background_color = color.toFTXUIColor();
        p.foreground_color = utils::getContrastColor(color);
    });

    double child_offset = angle_offset;
    for (size_t i = 0; i < slice.children.size(); ++i) {
        const auto& child = slice.children[i];
        utils::RGB child_color = utils::dim(color, 0.85 - (0.15 * i));

        RenderSliceRecursive(c, bonsai, child, child_offset, depth + 1, cx, cy, base_radius, child_color);
        child_offset += child.percent / 100.0 * 360.0;
    }
}

}


namespace ui {

Component RenderPieChart(const ui::PieRing& ring, const std::string& center_label) {
    return Renderer([ring, center_label] {
        return canvas([ring, center_label](Canvas& c) {
            int width = c.width();
            int height = c.height();

            int cx = width / 2;
            int cy = height / 2;

            int radius = std::min(width, height) / 6;

            auto sorted_ring = ring;
            std::sort(sorted_ring.slices.begin(), sorted_ring.slices.end(), [](const RingSlice& a, const RingSlice& b) {
                return a.percent > b.percent;
            });
            
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

            double angle_offset = 0.0;
            for (size_t j = 0; j < sorted_ring.slices.size(); ++j) {
                const auto& slice = sorted_ring.slices[j];
                utils::RGB color = utils::base_palette[j % utils::base_palette.size()];

                RenderSliceRecursive(c, bonsai, slice, angle_offset, 0, cx, cy, radius, color);
                angle_offset += slice.percent / 100.0 * 360.0;
            }

        }) | flex | border;
    });
}

}