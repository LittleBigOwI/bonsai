# pragma once

#include "../../include/core/app_data.hpp"
#include "../../include/core/scanner.hpp"

#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>

using namespace ftxui;

class BonsaiPie {
public:
    static Component pie(std::shared_ptr<AppData::BonsaiData> data);
    static void worker(ScreenInteractive* screen, std::shared_ptr<AppData::BonsaiData> data, Scanner* scanner, const fs::path& default_path);

    static void stop(std::shared_ptr<AppData::BonsaiData> data);

private:
    struct EntryInfo {
        std::filesystem::path path;
        uint64_t size;
        
        bool is_dir;
        int depth;
    };

    static void collectEntries(const fs::path& dir, std::vector<EntryInfo>& entries, int current_depth, int max_depth, Scanner* scanner);
    static void drawAngledBlockEllipseRingOffset(Canvas& c, int x1, int y1, int r1, int r2, int r3, double starting_angle, double angle, const Canvas::Stylizer& s);
};