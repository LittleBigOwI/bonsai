#include "sidebar.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <string>
#include <vector>
#include <filesystem>

#define SIDEBAR_FOLDER_ICON ""
#define SIDEBAR_SELECTED_FOLDER_ICON ""
#define SIDEBAR_FILE_ICON ""
#define SIDEBAR_SELECTED_FILE_ICON ""

namespace ui {
    using namespace ftxui;
    namespace fs = std::filesystem;

    class SidebarComponent : public ComponentBase {
    public:
        SidebarComponent(const std::string& path, int width) : path_(path), width_(width) {
            using Entry = std::pair<std::string, bool>;
            std::vector<Entry> temp_entries;
            
            try {
                for (const auto& entry : fs::directory_iterator(path_)) {
                    std::string name = entry.path().filename().string();
                    bool is_dir = entry.is_directory();
                    temp_entries.emplace_back(name, is_dir);
                }
            } catch (const std::exception& e) {
                entries_.push_back("Error reading directory");
                is_directory_.push_back(false);
                selected_ = 0;
                return;
            }

            std::sort(temp_entries.begin(), temp_entries.end(), [](const Entry& a, const Entry& b) {
                if (a.second != b.second)
                    return a.second > b.second;
                return a.first < b.first;
            });

            for (const auto& [name, is_dir] : temp_entries) {
                entries_.push_back(name);
                is_directory_.push_back(is_dir);
            }

            selected_ = 0;

            MenuOption option;
            option.entries = &entries_;
            option.selected = &selected_;
            
            option.entries_option.animated_colors.foreground = AnimatedColorOption{
                .inactive = Color::White,
                .active = Color::Default
            };

            option.entries_option.transform = [this](const EntryState& entry) {
                const bool is_selected = entry.active;
                const bool is_dir = is_directory_[entry.index];

                std::string icon_str = is_selected
                    ? (is_dir ? SIDEBAR_SELECTED_FOLDER_ICON : SIDEBAR_SELECTED_FILE_ICON)
                    : (is_dir ? SIDEBAR_FOLDER_ICON : SIDEBAR_FILE_ICON);

                auto icon = text(" " + icon_str + " ");
                auto label = text(entry.label);

                auto row = hbox({
                    icon,
                    text(" "),
                    label,
                });

                if (is_selected) {
                    row = row | bgcolor(Color::White) | color(Color::Black);
                } else {
                    row = row | bgcolor(Color::Default) | color(Color::White);
                }

                return row;
            };

            menu_ = Menu(&entries_, &selected_, option);
            Add(menu_);
        }

        Element OnRender() override {
            return vbox({
                text("Explorer") | bold | center,
                separator(),
                menu_->Render(),
            }) | border | size(WIDTH, EQUAL, width_);
        }

    private:
        int width_;
        std::string path_;

        std::vector<bool> is_directory_;
        std::vector<std::string> entries_;

        int selected_;
        Component menu_;
    };

    Component RenderSidebar(const std::string& path, int width) {
        return Make<SidebarComponent>(path, width);
    }
}
