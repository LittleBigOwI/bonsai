#include "../include/core/scanner.hpp"

#include "../include/ui/menu.hpp"

#include <ftxui/component/screen_interactive.hpp>

#include <iostream>
#include <thread>

#define DEFAULT_PATH "/"

int main() {
    using namespace ftxui;

    // Launch scanner
    Scanner scanner = Scanner(DEFAULT_PATH);
    std::thread scanner_thread([&scanner]() { scanner.scan(); });

    auto screen = ScreenInteractive::Fullscreen();

    // Init shared menu & pie data
    int selected = 0;
    auto data = std::make_shared<BonsaiMenu::BonsaiMenuData>();
    
    data->entries = std::make_shared<std::vector<BonsaiMenu::BonsaiMenuEntry>>();
    data->labels = std::make_shared<std::vector<std::string>>();
    data->path = std::make_shared<std::string>(DEFAULT_PATH);

    MenuOption option;

    /* Init menu:
    - Use a container to keep focus through the entire render.
      If a menu component is embeded in a slew of elements, it becomes static)
    */
    auto menu_component = BonsaiMenu::menu(&screen, data, &selected, DEFAULT_PATH, option);
    auto menu_container = Container::Vertical({menu_component});

    std::thread menu_thread(BonsaiMenu::worker, &screen, data, DEFAULT_PATH);

    // Init main UI
    auto window = Renderer(menu_container, [&] {
        std::lock_guard<std::mutex> lock(data->entries_mutex);
        
        return hbox({
            vbox({
                text(" "),
                // menu_title("Explorer")->Render(),
                text(" "),
                separator(),
                hbox({
                    menu_component->Render() | size(WIDTH, EQUAL, SIDEBAR_WIDTH) | frame,
                    text(" ")
                })
            }),
            separator(),
        });
    });

    // Render UI
    screen.Loop(window);
    
    scanner.stop();
    scanner_thread.join();

    BonsaiMenu::stop(data);
    menu_thread.join();

    return 0;
}
