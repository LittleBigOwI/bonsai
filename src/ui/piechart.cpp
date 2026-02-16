#include "../../include/config/config.hpp"
#include "../../include/ui/piechart.hpp"

#include <iostream>
#include <cmath>
#include <map>

void BonsaiPie::drawAngledBlockEllipseRingOffset(Canvas& c, int x1, int y1, int r1, int r2, int r3, double starting_angle, double angle, const Canvas::Stylizer &s) {
    int x = -r1;
    int y = 0;
    int e2 = r2;
    int dx = (1 + 2 * x) * e2 * e2;
    int dy = x * x;
    int err = dx + dy;

    double start_angle_rad = starting_angle * M_PI / 180.0;
    double max_angle_rad = angle * M_PI / 180.0;
    int r3_squared = r3 * r3;

    auto within_angular_ring = [&](int dx, int dy)
    {
        double angle_rad = atan2(dy * r1, dx * r2);
        if (angle_rad < 0)
            angle_rad += 2 * M_PI;

        double rel_angle = angle_rad - start_angle_rad;
        if (rel_angle < 0)
            rel_angle += 2 * M_PI;

        if (rel_angle > max_angle_rad)
            return false;

        double norm_x = static_cast<double>(dx) / r1;
        double norm_y = static_cast<double>(dy) / r2;
        double distance_squared = norm_x * norm_x + norm_y * norm_y;

        double inner_radius_ratio = static_cast<double>(r3) / std::min(r1, r2);
        return distance_squared >= inner_radius_ratio * inner_radius_ratio;
    };

    do
    {
        for (int xx = x1 + x; xx <= x1 - x; ++xx)
        {
            int dx = xx - x1;
            if (within_angular_ring(dx, y))
                c.DrawBlock(xx, y1 + y, true, s);
            if (within_angular_ring(dx, -y))
                c.DrawBlock(xx, y1 - y, true, s);
        }

        e2 = 2 * err;
        if (e2 >= dx)
        {
            x++;
            err += dx += 2 * r2 * r2;
        }
        if (e2 <= dy)
        {
            y++;
            err += dy += 2 * r1 * r1;
        }
    } while (x <= 0);

    while (y++ < r2)
    {
        if (within_angular_ring(0, y))
            c.DrawBlock(x1, y1 + y, true, s);
        if (within_angular_ring(0, -y))
            c.DrawBlock(x1, y1 - y, true, s);
    }
}

void BonsaiPie::collectEntries(const fs::path& dir, std::vector<EntryInfo>& entries, int current_depth, int max_depth, Scanner* scanner) {
    if (current_depth > max_depth) return;

    try {
        for (const auto& entry : fs::directory_iterator(dir)) {
            if (fs::is_symlink(entry.path()))
                continue;

            EntryInfo info;
            info.path = entry.path();
            info.is_dir = entry.is_directory();
            info.depth = current_depth;

            if (info.is_dir) {
                info.size = scanner->get(entry.path());
                entries.push_back(info);
                collectEntries(entry.path(), entries, current_depth + 1, max_depth, scanner);
            
            } else if (entry.is_regular_file()) {
                info.size = entry.file_size();
                entries.push_back(info);
            }
        }
    } catch (const fs::filesystem_error&) {}
}

void BonsaiPie::worker(ScreenInteractive* screen, std::shared_ptr<AppData::BonsaiData> data, Scanner* scanner, const fs::path& default_path) {
    int passes = 0;
    
    while (true) {
        std::vector<EntryInfo> entries;

        fs::path current_path;
        {
            std::lock_guard<std::mutex> lock(data->menu_mutex);
            current_path = *data->path;
        }

        // Parse current path with a max depth of 3
        collectEntries(current_path, entries, 0, 3, scanner);

        /* Sort:
        - Depth first: lower depth is higer priority
        - Type second: directories have a higher priority over files
        - Size third: larger sizes have a higher priority over lower ones
        
        - Maybe we could use a priority queue here? idk.
        */
        std::sort(entries.begin(), entries.end(), [](const EntryInfo& a, const EntryInfo& b) {
            if (a.depth != b.depth)
                return a.depth < b.depth;
            if (a.is_dir != b.is_dir)
                return a.is_dir > b.is_dir;
            return a.size > b.size;
        });

        std::vector<AppData::BonsaiPieEntry> slices;

        // Get size of current dir
        uint64_t root_size = scanner->get(current_path);
        Config cfg = Config::get();

        int i = 0;

        std::map<std::string, int> layer_offsets_per_parent;
        std::map<std::string, int> slice_offsets;
        
        for(auto& entry : entries) {
            if(root_size <= 0) {
                continue;
            }

            if(entry.depth != 0 && entry.depth != 1) {
                continue;
            }

            if(entry.size > root_size) {
                continue;
            }

            int inner_hole_radius = 10;
            int inner_radius = inner_hole_radius + ((entry.depth + 1) * 20);
            int outer_radius = inner_radius + 20;

            int occupancy = entry.size * 100 / root_size;
            int sweep = occupancy * 360 / 100;

            if(occupancy < cfg.CHART_MAX_SIZE_THRESHOLD_PERCENTAGE) {
                continue;
            }

            std::array<int, 3UL> color = cfg.CHART_COLORS[i % cfg.CHART_COLORS.size()];

            AppData::BonsaiPieEntry slice;
            slice.color = Color::RGB(color[0], color[1], color[2]);
            slice.inner_radius = inner_radius;
            slice.outer_radius = outer_radius;
            slice.label = "";
            slice.sweep = sweep;

            if(entry.depth == 0) {
                slice.offset_angle = layer_offsets_per_parent["root"];
                layer_offsets_per_parent["root"] += sweep;
            } else {
                std::string parent = entry.path.parent_path().string();
                slice.offset_angle = slice_offsets[parent] + layer_offsets_per_parent[parent];
                layer_offsets_per_parent[parent] += sweep;
            }

            slice_offsets[entry.path.string()] = slice.offset_angle;
            slices.push_back(slice);

            i++;
        }

        // Publish results
        {
            std::lock_guard<std::mutex> lock(data->pie_mutex);
            *data->pie_entries = std::move(slices);
        }

        // Update render
        screen->PostEvent(Event::Custom);

        /* Bug fix:
        - On first render the pie will be updated with new size but not new folders
        - Probably caused by a desync between menu and pie threads? idk.
        - This forces a single extra pass/iteration (recompute everything on the first scan complete)
        */
        if(passes == 0 && scanner->isDone()) {
            passes++;
            continue;
        }

        // Sleep until woken up or keep going if scanner hasn't completed
        if(!scanner->isDone()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }


        {
            std::unique_lock<std::mutex> lock(data->pie_mutex);

            data->cv.wait(lock, [&] {
                return data->pie_path_changed || data->stop;
            });

            if (data->stop)
                break;

            if (data->pie_path_changed)
                data->pie_path_changed = false;
        }
    }
}

Component BonsaiPie::pie(std::shared_ptr<AppData::BonsaiData> data) {
    return Renderer([data] {
        return canvas([data](Canvas& c) {
            int w = c.width();
            int h = c.height();

            std::vector<AppData::BonsaiPieEntry> entries;
            {
                std::lock_guard<std::mutex> lock(data->pie_mutex);
                entries = *data->pie_entries;
            }

            for (AppData::BonsaiPieEntry entry : entries) {
                BonsaiPie::drawAngledBlockEllipseRingOffset(
                    c,
                    w / 2,
                    h / 2,
                    entry.outer_radius,
                    entry.outer_radius,
                    entry.inner_radius,
                    entry.offset_angle,
                    entry.sweep,
                    [entry](ftxui::Pixel &p){ p.foreground_color = entry.color; }
                );
                c.DrawText(w/2, h/2, entry.label);
            }

        }) | flex;
    });
}