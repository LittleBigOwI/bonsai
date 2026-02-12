#include "../../include/ui/piechart.hpp"

#include <cmath>

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
                if (it.depth() > 3) {
                    it.disable_recursion_pending();
                    continue;
                }

                if (it->is_directory(ec) && !ec) {
                    scanned_dirs.push_back(it->path());
                }
            }
        }

        /* What we have to do here:
        - recuperate current directory size with scanner map
        - recuperate all depth scanned directory sizes
        - for each "depth" of directory
          get get sizes of dirs in that depth
          get their percentage of use of the current dir
          get the angle that corresponds to that percentage (we could just compute the angle straight away)
        - each depth of directory corresponds to one ring on the canvas
        - inner ring is the closest depth
        - outer rings are the further depths
        - sort the scanned dirs by depth and by size descending
        - for each computed angle, draw a part of the final pie chart of that angle
        - inner rings decide color
        - outer rings dim color of parent directory
        - goal is to match the baobab piechart
        */

        passes++;
        if (data->pie_entries->empty()) {
            data->pie_entries->push_back(AppData::BonsaiPieEntry{"", Color::Black, 0, 0, 0, 0});
        }

        if(passes % 2 == 0) {
            (*data->pie_entries)[0].label = "true";
        } else {
            (*data->pie_entries)[0].label = "false";
        }

        {
            // Sleep until woken up
            std::unique_lock<std::mutex> lock(data->cv_mutex);
            data->cv.wait(lock, [&]{ return data->pie_path_changed || data->stop; });

            if (data->stop) break;
            data->pie_path_changed = false;
        }
    }
}

Component BonsaiPie::pie(std::shared_ptr<AppData::BonsaiData> data) {
    return Renderer([data] {
        return canvas([data](Canvas& c) {
            int w = c.width();
            int h = c.height();

            for (AppData::BonsaiPieEntry entry : (*data->pie_entries)) {
                // draw_angled_block_ellipse_ring_offset(
                //     c,
                //     w / 2,
                //     h / 2,
                //     si.outer_radius,
                //     si.outer_radius,
                //     si.inner_radius,
                //     si.offset_angle,
                //     si.sweep,
                //     [si](ftxui::Pixel &p){ p.foreground_color = si.color; }
                // );
                c.DrawText(w/2, h/2, entry.label);
            }

        }) | flex;
    });
}