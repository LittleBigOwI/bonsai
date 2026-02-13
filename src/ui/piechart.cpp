#include "../../include/ui/piechart.hpp"

#include <iostream>
#include <cmath>
#include <map>

void BonsaiPie::drawAngledBlockEllipseRingOffset(Canvas& c, int x1, int y1, int r1, int r2, int r3, double starting_angle, double angle, const Canvas::Stylizer& s) {
    const int steps = 360;

    double start_rad = starting_angle * M_PI / 180.0;
    double max_rad = angle * M_PI / 180.0;

    for (int i = 0; i <= steps; ++i) {
        double theta = (double)i / steps * max_rad + start_rad;

        double cos_theta = std::cos(theta);
        double sin_theta = std::sin(theta);

        int max_radius = std::max(r1, r2);
        for (double t = static_cast<double>(r3) / std::min(r1, r2); t <= 1.0; t += 0.009) {
            int dx = static_cast<int>(std::round(r1 * t * cos_theta));
            int dy = static_cast<int>(std::round(r2 * t * sin_theta));

            c.DrawBlock(x1 + dx, y1 + dy, true, s);
        }
    }
}

void BonsaiPie::worker(ScreenInteractive* screen, std::shared_ptr<AppData::BonsaiData> data, Scanner* scanner, const fs::path& default_path) {
    int passes = 0;
    while (true) {
        std::vector<std::filesystem::path> scanned_dirs;

        // Perform a single recursive scan (only directories)
        std::error_code ec;
        std::filesystem::recursive_directory_iterator it(
            *data->path,
            std::filesystem::directory_options::skip_permission_denied,
            ec
        );

        if (!ec) {
            for (; it != std::filesystem::recursive_directory_iterator(); it.increment(ec)) {
                if (ec) { ec.clear(); continue; }

                // Limit depth
                if (it.depth() > 1) {
                    it.disable_recursion_pending();
                    continue;
                }

                if (it->is_directory(ec) && !ec) {
                    scanned_dirs.push_back(it->path());
                }
            }
        }

        /* What we have to do here:
        - recuperate current directory size with scanner map with: scanner->get(data->path) and std::lock_guard<std::mutex> lock(data->menu_mutex);
        - recuperate all depth scanned directory sizes with: scanned_dirs for each scanner->get(scanned_dir)
        - for each "depth" of directory
          get get sizes of dirs in that depth (already done)
          get their percentage of use of the current dir
          get the angle that corresponds to that percentage (we could just compute the angle straight away) 100% is 360 deg ofc.
        - each depth of directory corresponds to one ring on the canvas
        - inner ring is the closest depth to current dir
        - outer rings are the further depths
        - sort the scanned dirs by depth then by size descending
        - inner rings decide color
        - outer rings dim color of parent directory
        - for each computed angle, compute a part of the final pie chart of that angle with void BonsaiPie::drawAngledBlockEllipseRingOffset(Canvas& c, int x1, int y1, int r1, int r2, int r3, double starting_angle, double angle, const Canvas::Stylizer& s);
        - add all results in a vector of BonsaiPieEntry
        - goal is to match the baobab piechart
        */

        std::vector<AppData::BonsaiPieEntry> entries;

        uint64_t root_size = 0;
        {
            std::lock_guard<std::mutex> lock(data->menu_mutex);
            root_size = scanner->get(*data->path);
        }

        int usr_index = 0;
        for(int i = 0; i < scanned_dirs.size(); i++) {
            if(scanned_dirs[i] == "/home") {
                usr_index = i;
            }
        }        

        Color slice_color = Color::Red1;
        uint64_t first_dir_size = scanner->get(scanned_dirs[usr_index]); 
        int occupancy = first_dir_size * 100 / root_size;
        int sweep = occupancy * 360 / 100;
        
        AppData::BonsaiPieEntry entry;
        entry.color = slice_color;
        entry.inner_radius = 15;
        entry.outer_radius = 30;
        entry.label = scanned_dirs[usr_index];
        entry.offset_angle = 0;
        entry.sweep = sweep;

        entries.push_back(entry);

        // Publish results
        {
            std::lock_guard<std::mutex> lock(data->pie_mutex);
            *data->pie_entries = std::move(entries);
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

            {
                std::lock_guard<std::mutex> lock(data->pie_mutex);
                
                for (AppData::BonsaiPieEntry entry : (*data->pie_entries)) {
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
            }

        }) | flex;
    });
}