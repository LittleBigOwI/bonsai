#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

#include <functional>
#include <string>

namespace ui {
    using namespace ftxui;

    class ErrorModalComponent : public ComponentBase {
    public:
        ErrorModalComponent(const std::string& title, const std::string& close) : title_(title), close_(close), on_close_([] {}) {
            build();
        }

        void setOnCloseCallback(std::function<void()> onClose) {
            on_close_ = std::move(onClose);
        }

        void setStatusCode(int status) {
            status_ = status;
        }

        bool OnEvent(Event event) override {
            if (event != Event::Escape)
                return ComponentBase::OnEvent(event);
            
            on_close_();
            return true;
        }

    private:
        int status_;

        std::string title_;
        std::string close_;

        std::function<void()> on_close_;
        
        Component container_;

        void build() {
            auto button = Container::Horizontal({
                Button(close_, [this]{ on_close_(); }, ButtonOption::Ascii())
            });

            auto layout = Container::Vertical({
                button,
            });

            Add(Renderer(layout, [=] {
                return vbox({
                    text(""),
                    text("  " + title_ + "  ") | bold | center | color(Color::Red),
                    text(" Error Code: " + std::to_string(status_) + " ") | bold | center | color(Color::Red),
                    text(""),
                    button->Render() | center | color(Color::Red)
                }) | size(WIDTH, GREATER_THAN, 50) | border | color(Color::GrayDark);
            }));
        }
    };

    inline std::shared_ptr<ErrorModalComponent> error(const std::string& title, const std::string& close) {
        return std::make_shared<ErrorModalComponent>(title, close);
    }
}
