#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <iostream>

int main() {
    using namespace ftxui;

    Element document = hbox({
            text("left") | border,
            text("middle") | border | flex,
            text("right") | border
    });

    auto screen = Screen::Create(
            Dimension::Full(),
            Dimension::Fit(document)
    );

    Render(screen, document);
    screen.Print();
}
