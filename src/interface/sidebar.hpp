#pragma once

#include "../utils/dataformat.hpp"
#include "../core/scanner.hpp"

#include <ftxui/component/component.hpp>

namespace ui {
    using namespace ftxui;

    class SidebarComponent : public ComponentBase {
    public:
        SidebarComponent(ScanSnapshot& snapshot, int width, const std::string& path): snapshot_(snapshot), width_(width), path_(path) {
            build();
            setup();
        }

        void setOnChangeCallback(std::function<void(const std::string&)> cb) {
            on_change_callback_ = std::move(cb);
        }

        void setOnEnterCallback(std::function<void(const std::string&)> cb) {
            on_enter_callback_ = std::move(cb);
        }

        void rebuild() {
            build();
        }

    private:
        int width_;
        int selected_ = 0;
        std::string path_;

        std::function<void(const std::string&)> on_change_callback_;
        std::function<void(const std::string&)> on_enter_callback_;

        std::vector<std::shared_ptr<TreeNode>> sorted_children_;
        std::vector<std::string> entries_;
        std::vector<bool> is_directory_;
        
        ScanSnapshot& snapshot_;
        Component menu_;

        std::shared_ptr<TreeNode> getSelectedNode() {
            if (selected_ < 0 || selected_ >= (int)entries_.size())
                return nullptr;

            if (entries_[selected_] == "..")
                return nullptr;

            int index = selected_ - (path_ != "/" ? 1 : 0);
            if (index < 0 || index >= (int)sorted_children_.size())
                return nullptr;

            return sorted_children_[index];
        }

        void build() {
            entries_.clear();
            is_directory_.clear();
            sorted_children_.clear();

            std::shared_ptr<TreeNode> node = Scanner::getNode(path_, snapshot_);
            if (!node) return;

            std::vector<std::shared_ptr<TreeNode>> children;

            if (path_ != "/") {
                entries_.push_back("..");
                is_directory_.push_back(true);
            }

            for (auto& child : node->children) {
                children.push_back(child);
            }

            std::sort(children.begin(), children.end(), [](const auto& a, const auto& b) {
                if (a->is_dir != b->is_dir) return a->is_dir > b->is_dir;
                if (a->size != b->size) return a->size > b->size;
                return a->name < b->name;
            });

            for (auto& child : children) {
                entries_.push_back(child->name);
                is_directory_.push_back(child->is_dir);
            }

            sorted_children_ = std::move(children);
        }

        void setup() {
            MenuOption option;
            option.entries_option.animated_colors.foreground = AnimatedColorOption{ 
                .inactive = Color::White, 
                .active = Color::Default
            };
            
            option.entries_option.transform = [this](const EntryState& entry) {
                const bool is_selected = entry.active;
                const bool is_dir = is_directory_[entry.index];
                
                const Config& config = Config::get();
                std::string label = entries_[entry.index];
                std::string icon_str = label == ".." ? config.SIDEBAR_BACK_ICON : (is_selected
                    ? (is_dir ? config.SIDEBAR_SELECTED_FOLDER_ICON : config.SIDEBAR_SELECTED_FILE_ICON)
                    : (is_dir ? config.SIDEBAR_FOLDER_ICON : config.SIDEBAR_FILE_ICON)
                );

                std::shared_ptr<TreeNode> node = nullptr;
                if (label != "..") {
                    auto parent_node = (path_ == "/" ? snapshot_.root : Scanner::getNode(path_, snapshot_));
                    node = sorted_children_[entry.index - (path_ != "/" ? 1 : 0)];
                }

                auto row = hbox({
                    text(" " + icon_str + " " + label) | size(WIDTH, LESS_THAN, this->width_ - 15),
                    filler(),
                    node ? text(DataFormat::toReadable(node->size)) | color(Color::GrayDark) : text("")
                });

                return is_selected
                    ? (row | bgcolor(Color::White) | color(Color::Black))
                    : (row | bgcolor(Color::Default) | color(Color::White));
            };

            option.on_change = [this]() {
                auto node = getSelectedNode();
                if(on_change_callback_) {
                    on_change_callback_(node ? node->cached_full_path : "");
                } 
            };

            option.on_enter = [this]() {
                const std::string& selected_entry = entries_[selected_];
                bool is_dir = is_directory_[selected_]; 
                if (!is_dir) return;

                if (selected_entry == "..") {
                    if (path_ != "/") {
                        path_ = fs::path(path_).parent_path().string();
                        if (path_.empty()) path_ = "/";
                    }
                } else {
                    path_ = fs::path(path_) / selected_entry;
                }

                build();

                if (on_enter_callback_)
                    if(on_change_callback_) {
                        auto node = getSelectedNode();
                        on_change_callback_(node ? node->cached_full_path : "");
                    }

                    on_enter_callback_(path_);
            };

            menu_ = Menu(&entries_, &selected_, option);
            Add(menu_);
        }
    };

    inline std::shared_ptr<SidebarComponent> sidebar(ScanSnapshot& snapshot, int width, const std::string path) {
        return std::make_shared<SidebarComponent>(snapshot, width, path);
    }
}