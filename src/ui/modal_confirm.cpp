#include "../../include/ui/modal_confirm.hpp"

Component BonsaiModalConfirm::confirm(std::function<void ()> action, std::function<void ()> exit) {
    auto component = Container::Horizontal({
        Button("confirm", action, ButtonOption::Ascii()) | bgcolor(Color::Default) | color(Color::SpringGreen2),
        Button("cancel", exit, ButtonOption::Ascii()) | bgcolor(Color::Default) | color(Color::IndianRed),
    });

    component |= Renderer([&](Element inner) {
        return vbox({
            text(""),
            text("Are you sure you want to delete the following file / folder?") | center,
            text("This action is irreversible.") | center,
            text(""),
            inner | center,
            text(""),

        }) | size(WIDTH, GREATER_THAN, 70) | border;
    });
    return component;
}