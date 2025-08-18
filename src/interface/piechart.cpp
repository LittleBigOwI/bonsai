#include "piechart.hpp"
#include "../widgets/canvas_ext.hpp"

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

        bonsai.DrawAngledBlockCircleRingOffset(cx, cy, outer_r, inner_r,
                                            angle_offset + .5, angle,
                                            color.toFTXUIColor());

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

    ui::RingSlice BuildSliceRecursive(
        const ScanSnapshot& snapshot,
        const std::string& path,
        int depth,
        int max_depth,
        double min_percent)
    {
        ui::RingSlice slice;
        slice.label = std::filesystem::path(path).filename().string();

        uintmax_t size = 0;
        {
            // auto it = snapshot.path_stats.find(path);
            // if (it != snapshot.path_stats.end())
            //     size = it->second.size;
        }

        double total_size = static_cast<double>(snapshot.total_size.load());
        slice.percent = total_size > 0 ? (size * 100.0 / total_size) : 0.0;

        if (depth >= max_depth || slice.percent < min_percent)
            return slice;

        // Gather children
        std::vector<std::pair<std::string, uintmax_t>> children;

        {
            // for (auto& [child_path, stats] : snapshot.path_stats) {
            //     if (child_path.rfind(path, 0) == 0 && child_path != path) {
            //         auto rel = std::filesystem::path(child_path).lexically_relative(path);
            //         if (rel.has_parent_path() && rel.parent_path() == "")
            //             children.emplace_back(rel.string(), stats.size);
            //     }
            // }
        }

        // Sort by size descending
        std::sort(children.begin(), children.end(),
                [](auto& a, auto& b) { return a.second > b.second; });

        // Add slices recursively
        for (auto& [name, child_size] : children) {
            double percent = total_size > 0 ? (child_size * 100.0 / total_size) : 0.0;
            if (percent < min_percent) continue;

            std::string full_path = std::filesystem::path(path) / name;
            slice.children.push_back(BuildSliceRecursive(snapshot, full_path, depth + 1, max_depth, min_percent));
        }

        return slice;
    }
}

namespace ui {

    PieChart::PieChart(PieRing ring, ScanSnapshot& snapshot): ring_(std::move(ring)), snapshot_(snapshot) {}

    Component BuildPieRingFromSnapshot(ScanSnapshot& snapshot, const std::string& root_dir, int max_rings, double min_percent) {
        PieRing ring;
        ring.slices.push_back(BuildSliceRecursive(snapshot, root_dir, 0, max_rings, min_percent));
        return Make<PieChart>(std::move(ring), snapshot);
    }

    Element PieChart::OnRender() {
        renderCount_++;

        PieRing ring;
        ring.slices.push_back(BuildSliceRecursive(snapshot_, "/", 0, 4, 1.0));
        ring_ = ring;

        return vbox({
            text("PieChart: " + std::to_string(renderCount_)) | center,
            separator(),
            canvas([&](Canvas& c) {
                int width = c.width();
                int height = c.height();

                int cx = width / 2;
                int cy = height / 2;
                int radius = std::min(width, height) / 6;

                auto sorted_ring = ring_;
                std::sort(sorted_ring.slices.begin(), sorted_ring.slices.end(),
                        [](const RingSlice& a, const RingSlice& b) {
                            return a.percent > b.percent;
                        });

                BonsaiCanvas bonsai(c);

                std::string stats = std::to_string(snapshot_.total_files) + " files\n" +
                                    std::to_string(snapshot_.total_dirs) + " dirs";

                size_t split = stats.find("\n");
                if (split != std::string::npos) {
                    std::string line1 = stats.substr(0, split);
                    std::string line2 = stats.substr(split + 1);
                    c.DrawText(cx - int(line1.size()) / 2, cy - 1, line1);
                    c.DrawText(cx - int(line2.size()) / 2, cy + 1, line2);
                } else {
                    c.DrawText(cx - int(stats.size()) / 2, cy, stats);
                }

                double angle_offset = 0.0;
                for (size_t j = 0; j < sorted_ring.slices.size(); ++j) {
                    const auto& slice = sorted_ring.slices[j];
                    utils::RGB color = utils::base_palette[j % utils::base_palette.size()];

                    RenderSliceRecursive(c, bonsai, slice, angle_offset, 0, cx, cy, radius, color);
                    angle_offset += slice.percent / 100.0 * 360.0;
                }
            }) | flex
        }) | flex | border;
    }

    Component MakePieChart(PieRing ring, ScanSnapshot& snapshot) {
        return Make<PieChart>(std::move(ring), snapshot);
    }

}
