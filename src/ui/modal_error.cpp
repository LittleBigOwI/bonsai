#include "../../include/ui/modal_error.hpp"

Component BonsaiModalError::error(const std::string& subtitle, std::function<void ()> exit) {
    auto component = Container::Horizontal({
        Button("exit", exit, ButtonOption::Ascii()) | bgcolor(Color::Default),
    });

    component |= Renderer([&](Element inner) {
        return vbox({
            text(""),
            text("Error during file / folder removal.") | center,
            text(subtitle) | center | color(Color::IndianRed),
            text(""),
            inner | center,
            text(""),

        }) | size(WIDTH, GREATER_THAN, 70) | border;
    });
    return component;
}