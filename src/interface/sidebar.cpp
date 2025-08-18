#include "sidebar.hpp"
#include "../utils/dataformat.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

#include <filesystem>
#include <functional>
#include <string>
#include <vector>
#include <algorithm>

#define SIDEBAR_FOLDER_ICON ""
#define SIDEBAR_SELECTED_FOLDER_ICON ""
#define SIDEBAR_FILE_ICON ""
#define SIDEBAR_SELECTED_FILE_ICON ""

namespace ui {
    using namespace ftxui;
    namespace fs = std::filesystem;

    class SidebarComponent : public ComponentBase {
    public:
        SidebarComponent(const std::string& path, int width, ScanSnapshot& snapshot)
        : path_(fs::absolute(path).string()), width_(width), snapshot_(snapshot) {
            SetupMenu();
            OnEvent(Event::Custom);
        }

        bool OnEvent(Event event) override {
            if (event == Event::Custom) {
                RebuildEntries();
                return true;
            }
            return ComponentBase::OnEvent(event);
        }

        Element OnRender() override {
            renderCount++;
            return vbox({
                text("Explorer: " + std::to_string(renderCount)) | bold | center,
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
        ScanSnapshot& snapshot_;

        int renderCount = 0;

        void SetupMenu() {
            MenuOption option;

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

                auto row = hbox({
                    text(" " + icon_str + " "),
                    text(" "),
                    text(label) | size(WIDTH, LESS_THAN, width_ - 10),
                    filler(),
                    label == ".."
                        ? text("")
                        : text(DataFormat::toReadable(
                            Scanner::getNode((fs::path(path_) / entries_[entry.index]).string(), this->snapshot_)->size
                        )) | color(Color::GrayDark)
                });

                return is_selected ? (row | bgcolor(Color::White) | color(Color::Black))
                                : (row | bgcolor(Color::Default) | color(Color::White));
            };

            option.on_enter = [this]() {
                const std::string& selected_entry = entries_[selected_];
                bool is_dir = is_directory_[selected_];
                if (!is_dir) return;

                fs::path new_path;
                if (selected_entry == "..") {
                    new_path = fs::path(path_).parent_path();
                } else {
                    selected_ = 1;
                    new_path = fs::path(path_) / selected_entry;
                }
                path_ = fs::absolute(new_path).string();

                RebuildEntries();
            };

            menu_ = Menu(&entries_, &selected_, option);
            Add(menu_);
        }

        void RebuildEntries() {
            using Entry = std::pair<std::string, bool>;
            entries_.clear();
            is_directory_.clear();

            std::vector<Entry> temp_entries;
            try {
                for (const auto& entry : fs::directory_iterator(path_)) {
                    std::string name = entry.path().filename().string();
                    bool is_dir = entry.is_directory();
                    temp_entries.emplace_back(name, is_dir);
                }
            } catch (...) {
                temp_entries.clear();
                temp_entries.emplace_back("Error reading directory", false);
                selected_ = 0;
            }

            // sort: dirs first, then name
            std::sort(temp_entries.begin(), temp_entries.end(), [](const Entry& a, const Entry& b) {
                if (a.second != b.second) return a.second > b.second;
                return a.first < b.first;
            });

            if (fs::path(path_) != fs::path(path_).root_path()) {
                entries_.push_back("..");
                is_directory_.push_back(true);
            }

            std::sort(temp_entries.begin(), temp_entries.end(),
                [this](const Entry& a, const Entry& b) {
                    fs::path path_a = fs::path(path_) / a.first;
                    fs::path path_b = fs::path(path_) / b.first;

                    uintmax_t size_a = Scanner::getNode(path_a.string(), snapshot_)->size;
                    uintmax_t size_b = Scanner::getNode(path_b.string(), snapshot_)->size;

                    if (size_a != size_b) return size_a > size_b;
                    if (a.second != b.second) return a.second > b.second;
                    return a.first < b.first;
                });

            for (const auto& [name, is_dir] : temp_entries) {
                entries_.push_back(name);
                is_directory_.push_back(is_dir);
            }

            if (selected_ >= (int)entries_.size()) selected_ = std::max(0, (int)entries_.size() - 1);
        }
    };

    Component RenderSidebar(const std::string& path, const int width, ScanSnapshot& snapshot) {
        return Make<SidebarComponent>(path, width, snapshot);
    }
}
