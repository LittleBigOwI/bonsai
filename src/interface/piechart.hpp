#pragma once

#include "../widgets/canvas_ext.hpp"
#include "../utils/dataformat.hpp"
#include "../utils/colors.hpp"
#include "../core/scanner.hpp"

#include <ftxui/component/component.hpp>

#include <algorithm>
#include <cmath>

#define CHART_MAX_SIZE_THRESHOLD_PERCENTAGE 2
#define CHART_MAX_GENERATIONS 4

namespace ui {
    using namespace ftxui;

    struct RingSlice {
        double percent;

        std::string path;
        std::string label;
        std::vector<RingSlice> children;
    };

    class PiechartComponent : public ComponentBase {
    public:
        PiechartComponent(ScanSnapshot& snapshot, const std::string& path): snapshot_(snapshot), path_(path), selected_path_("") {
            build();
            setup();
        }

        void setSelected(const std::string& path) {
            this->selected_path_ = path;
        }

        void setPath(const std::string& path) {
            this->path_ = path;
            this->build();
        }
    
    private:
        std::vector<RingSlice> slices;
        std::string selected_path_;
        std::string path_;
        
        ScanSnapshot& snapshot_;
        Element canvas_;

        double getPercentage(const std::string& path) {
            auto node = Scanner::getNode(path, snapshot_);
            if (!node) return 0.0;
            return (static_cast<double>(node->size) * 100.0) / Scanner::getNode(this->path_, snapshot_)->size;
        }

        RingSlice buildSlices(const std::shared_ptr<TreeNode>& node, int depth, int max_depth) {
            RingSlice slice;
            slice.path = node->cached_full_path;
            slice.label = node->name;
            slice.percent = getPercentage(node->cached_full_path.string());

            if (depth >= max_depth || slice.percent < CHART_MAX_SIZE_THRESHOLD_PERCENTAGE)
                return slice;

            for (auto& child : node->children) {
                double child_percent = getPercentage(child->cached_full_path.string());
                if (child_percent < CHART_MAX_SIZE_THRESHOLD_PERCENTAGE) continue;

                slice.children.push_back(buildSlices(child, depth + 1, max_depth));
            }

            std::sort(slice.children.begin(), slice.children.end(), [](const RingSlice& a, const RingSlice& b) { return a.percent > b.percent; });
            return slice;
        }

        void build() {
            slices.clear();

            auto root_node = Scanner::getNode(path_, snapshot_);
            if (!root_node) return;

            for (auto& child : root_node->children) {
                double percent = getPercentage(child->cached_full_path.string());
                if (percent < CHART_MAX_SIZE_THRESHOLD_PERCENTAGE) continue;

                slices.push_back(buildSlices(child, 0, CHART_MAX_GENERATIONS - 1));
            }
            std::sort(slices.begin(), slices.end(), [](const RingSlice& a, const RingSlice& b) { return a.percent > b.percent; });
        }

        void setupSlice(Canvas& c, BonsaiCanvas& bonsai, const ui::RingSlice& slice, double angle_offset, int depth, int cx, int cy, int base_radius, const utils::RGB& color) {
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

            c.DrawText((x - .5), y, slice.label, [color](Pixel &p) {
                p.background_color = color.toFTXUIColor();
                p.foreground_color = utils::getContrastColor(color);
            });

            double child_offset = angle_offset;
            for (size_t i = 0; i < slice.children.size(); ++i) {
                const auto& child = slice.children[i];
                utils::RGB child_color = utils::dim(color, 0.85 - (0.15 * i));

                setupSlice(c, bonsai, child, child_offset, depth + 1, cx, cy, base_radius, child_color);
                child_offset += child.percent / 100.0 * 360.0;
            }
        }

        void setup() {
            canvas_ = canvas([&](Canvas& c) {
                int width = c.width();
                int height = c.height();

                int cx = width / 2;
                int cy = height / 2;
                int radius = std::min(width, height) / 6;

                BonsaiCanvas bonsai(c);

                std::string stats = DataFormat::toReadable(Scanner::getNode(path_, snapshot_)->size, "\n");

                size_t split = stats.find("\n");
                std::string line1 = stats.substr(0, split);
                std::string line2 = stats.substr(split + 1);
                c.DrawText((cx - .5) - int(line1.size()) / 2, cy - 2, line1);
                c.DrawText((cx - .5) - int(line2.size()) / 2, cy + 2, line2);
                
                double angle_offset = 0.0;
                for (size_t j = 0; j < slices.size(); ++j) {
                    const auto& slice = slices[j];
                    utils::RGB color = utils::base_palette[j % utils::base_palette.size()];

                    if(slice.path == this->selected_path_) 
                        color = utils::white;

                    setupSlice(c, bonsai, slice, angle_offset, 0, cx, cy, radius, color);
                    angle_offset += slice.percent / 100.0 * 360.0;
                }
            }) | flex;

            auto component = Renderer([this] () {
                return canvas_;
            });

            Add(component);
        }
    };

    inline std::shared_ptr<PiechartComponent> piechart(ScanSnapshot& snapshot, const std::string& path) {
        return std::make_shared<PiechartComponent>(snapshot, path);
    }
}