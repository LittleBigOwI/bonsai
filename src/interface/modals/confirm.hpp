#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

#include <functional>
#include <string>

namespace ui {
    using namespace ftxui;

    class ComfirmModalComponent : public ComponentBase {
    public:
        ComfirmModalComponent(const std::string& title, const std::string& subtitle, const std::string& confirm, const std::string& close) : 
        title_(title), subtitle_(subtitle), confirm_(confirm), close_(close), on_confirm_([] {}), on_close_([] {}) {
            build();
        }

        void setOnConfirmCallback(std::function<void()> onConfirm) {
            on_confirm_ = std::move(onConfirm);
        }

        void setOnCloseCallback(std::function<void()> onClose) {
            on_close_ = std::move(onClose);
        }

        bool OnEvent(Event event) override {
            if (event != Event::Escape)
                return ComponentBase::OnEvent(event);
            
            on_close_();
            return true;
        }

    private:
        std::string title_;
        std::string subtitle_;
        std::string confirm_;
        std::string close_;

        std::function<void()> on_confirm_;
        std::function<void()> on_close_;
        
        Component container_;

        void build() {
            auto buttons = Container::Horizontal({
                Button(confirm_, [this]{ on_confirm_(); }, ButtonOption::Ascii()),
                Button(close_, [this]{ on_close_(); }, ButtonOption::Ascii()),
            });

            auto layout = Container::Vertical({
                buttons,
            });

            Add(Renderer(layout, [=] {
                return vbox({
                    text(""),
                    text("  " + title_ + "  ") | bold | center | color(Color::White),
                    text("  " + subtitle_ + "  ") | bold | center | color(Color::White),
                    text(""),
                    separator(),
                    buttons->Render() | center | color(Color::White)
                }) | size(WIDTH, GREATER_THAN, 50) | border | color(Color::GrayDark);
            }));
        }
    };

    inline std::shared_ptr<ComfirmModalComponent> confirm(const std::string& title, const std::string& subtitle, const std::string& confirm, const std::string& close) {
        return std::make_shared<ComfirmModalComponent>(title, subtitle, confirm, close);
    }
}
