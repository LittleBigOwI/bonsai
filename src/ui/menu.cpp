#include "../../include/ui/menu.hpp"

#include "../../include/utils/format.hpp"

void BonsaiMenu::worker(ScreenInteractive* screen, std::shared_ptr<BonsaiMenuData> data, const fs::path& default_path) {
    while (true) {
        std::vector<BonsaiMenuEntry> new_entries;
        std::vector<std::string> new_labels;

        // Always allow going back until DEFAULT_PATH is reached
        if (*data->path != default_path) {
            new_labels.push_back("..");
            new_entries.push_back(BonsaiMenuEntry{0, "..", "", true});
        }

        try {
            std::vector<BonsaiMenuEntry> unsorted_entries;

            // Parse current path and get relevant data (size, is_dir, path, etc.)
            for (const auto& entry : fs::directory_iterator(*data->path)) {
                BonsaiMenuEntry item;
                item.path = entry.path().string();
                item.label = entry.path().filename().string();
                item.is_dir = entry.is_directory();

                if (!item.is_dir) {
                    std::error_code ec;
                    item.size = entry.file_size(ec);
                    if (ec) item.size = 0;
                } else {
                    item.size = 0;
                }

                unsorted_entries.push_back(std::move(item));
            }

            /* Sort the aquired data:
            - Directories go first
            - Then compare size descending
            - Then compare name ascending
            */
            std::sort(unsorted_entries.begin(), unsorted_entries.end(), [](const BonsaiMenuEntry& a, const BonsaiMenuEntry& b) {
                if (a.is_dir != b.is_dir)
                    return a.is_dir;

                if (a.size != b.size)
                    return a.size > b.size;

                return a.label < b.label;
            });

            // Push the sorted data in the two array, in sync.
            for (auto& item : unsorted_entries) {
                new_labels.push_back(item.label);
                new_entries.push_back(std::move(item));
            }

        } catch (const fs::filesystem_error&) {}

        {
            // Update the menu data
            std::lock_guard<std::mutex> lock(data->entries_mutex);
            data->entries->swap(new_entries);
            data->labels->swap(new_labels);
        }

        // Update render
        screen->PostEvent(Event::Custom);

        {
            // Sleep until woken up
            std::unique_lock<std::mutex> lock(data->cv_mutex);
            data->cv.wait(lock, [&]{ return data->path_changed || data->stop; });
            if (data->stop) break;
            
            data->path_changed = false;
        }
    }
}

Component BonsaiMenu::menu(ScreenInteractive* screen, std::shared_ptr<BonsaiMenuData> data, int* selected, const fs::path& default_path, MenuOption options) {

    // Selected or unselected colors
    options.entries_option.animated_colors.foreground = AnimatedColorOption{
        .inactive = Color::White,
        .active = Color::Default
    };

    // Plot the entry data on the menu in a nice way
    options.entries_option.transform = [data](const EntryState& entry_state) {
        const auto& item = (*data->entries)[entry_state.index];
        const bool is_selected = entry_state.active;

        std::string icon_str;
        if (item.label == "..") {
            icon_str = SIDEBAR_BACK_ICON;
        } else {
            icon_str = is_selected ? (item.is_dir ? SIDEBAR_SELECTED_FOLDER_ICON : SIDEBAR_SELECTED_FILE_ICON)
                                   : (item.is_dir ? SIDEBAR_FOLDER_ICON          : SIDEBAR_FILE_ICON);
        }

        auto row = hbox({
            text(" " + icon_str + " " + item.label) | size(WIDTH, LESS_THAN, SIDEBAR_WIDTH - 15),
            filler(),
            text(FormatUtils::toReadableShort(item.size)) | color(Color::GrayDark)
        });

        return is_selected ? row | bgcolor(Color::White) | color(Color::Black)
                           : row | bgcolor(Color::Default) | color(Color::White);
    };

    // On selected: update data with a new path, and wake worker thread
    options.on_enter = [data, selected]() {
        std::string new_path;
        {
            std::lock_guard<std::mutex> lock(data->entries_mutex);
            if (*selected < 0 || *selected >= (int)data->entries->size() || !(*data->entries)[*selected].is_dir) return;
            new_path = (*data->entries)[*selected].path;

            // For ".." go up
            if ((*data->entries)[*selected].label == "..") {
                std::filesystem::path p(*data->path);
                new_path = p.parent_path().string();
            }

            *data->path = new_path;
        }

        {
            std::lock_guard<std::mutex> lock(data->cv_mutex);
            data->path_changed = true;
        }

        data->cv.notify_one();
    };

    // Menu is updated as labels update
    return Menu(data->labels.get(), selected, options);
}

void BonsaiMenu::stop(std::shared_ptr<BonsaiMenuData> data) {
    {
        std::lock_guard<std::mutex> lock(data->cv_mutex);
        data->stop = true;
    }
    data->cv.notify_one();
}