#include "sidebar.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <string>
#include <vector>
#include <filesystem>
#include <functional>

#define SIDEBAR_FOLDER_ICON ""
#define SIDEBAR_SELECTED_FOLDER_ICON ""
#define SIDEBAR_FILE_ICON ""
#define SIDEBAR_SELECTED_FILE_ICON ""

namespace ui {
    using namespace ftxui;
    namespace fs = std::filesystem;

    class SidebarComponent : public ComponentBase {
    public:
        SidebarComponent(const std::string& path, int width)
            : path_(fs::absolute(path).string()), width_(width) {
            LoadEntries();
        }

        Element OnRender() override {
            return vbox({
                text("Explorer") | bold | center,
                separator(),
                menu_->Render() | vscroll_indicator | frame,
            }) | border | size(WIDTH, EQUAL, width_);
        }

    private:
        int width_;
        std::string path_;

        std::vector<bool> is_directory_;
        std::vector<std::string> entries_;
        int selected_ = 0;

        Component menu_;

        void LoadEntries() {
            entries_.clear();
            is_directory_.clear();

            using Entry = std::pair<std::string, bool>;
            std::vector<Entry> temp_entries;

            try {
                for (const auto& entry : fs::directory_iterator(path_)) {
                    std::string name = entry.path().filename().string();
                    bool is_dir = entry.is_directory();
                    temp_entries.emplace_back(name, is_dir);
                }
            } catch (const std::exception& e) {
                temp_entries.emplace_back("Error reading directory", false);
                selected_ = 0;
            }

            std::sort(temp_entries.begin(), temp_entries.end(), [](const Entry& a, const Entry& b) {
                if (a.second != b.second)
                    return a.second > b.second;
                return a.first < b.first;
            });

            if (fs::path(path_) != fs::path(path_).root_path()) {
                entries_.push_back("..");
                is_directory_.push_back(true);
            }

            for (const auto& [name, is_dir] : temp_entries) {
                entries_.push_back(name);
                is_directory_.push_back(is_dir);
            }

            MenuOption option;
            option.entries = &entries_;
            option.selected = &selected_;

            option.entries_option.animated_colors.foreground = AnimatedColorOption{
                .inactive = Color::White,
                .active = Color::Default,
            };

            option.entries_option.transform = [this](const EntryState& entry) {
                const bool is_selected = entry.active;
                const bool is_dir = is_directory_[entry.index];

                std::string label = entry.label;
                std::string icon_str;

                if (label == "..") {
                    icon_str = "";
                } else {
                    icon_str = is_selected
                        ? (is_dir ? SIDEBAR_SELECTED_FOLDER_ICON : SIDEBAR_SELECTED_FILE_ICON)
                        : (is_dir ? SIDEBAR_FOLDER_ICON : SIDEBAR_FILE_ICON);
                }

                auto icon = text(" " + icon_str + " ");
                auto label_text = text(label);

                auto row = hbox({
                    icon,
                    text(" "),
                    label_text | size(WIDTH, LESS_THAN, width_ - 10),
                    filler(),
                    label == ".." ? text("") : text("?%") | color(Color::GrayDark)
                });

                if (is_selected) {
                    row = row | bgcolor(Color::White) | color(Color::Black);
                } else {
                    row = row | bgcolor(Color::Default) | color(Color::White);
                }

                return row;
            };

            option.on_enter = [this]() {
                const std::string& selected_entry = entries_[selected_];
                bool is_dir = is_directory_[selected_];

                if (is_dir) {
                    fs::path new_path;

                    if (selected_entry == "..") {
                        new_path = fs::path(path_).parent_path();
                    } else {
                        selected_ = 1;
                        new_path = fs::path(path_) / selected_entry;
                    }

                    path_ = fs::absolute(new_path).string();
                    LoadEntries();
                }
            };

            menu_ = Menu(&entries_, &selected_, option);
            Add(menu_);
        }
    };

    Component RenderSidebar(const std::string& path, int width) {
        return Make<SidebarComponent>(path, width);
    }
}
