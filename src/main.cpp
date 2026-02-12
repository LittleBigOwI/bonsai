#include "../include/config/config.hpp"
#include "../include/core/app_data.hpp"
#include "../include/core/scanner.hpp"
#include "../include/ui/piechart.hpp"
#include "../include/ui/menu.hpp"

#include <ftxui/component/screen_interactive.hpp>

#include <iostream>
#include <thread>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    using namespace ftxui;

    auto config = Config::get();
    auto screen = ScreenInteractive::Fullscreen();
    auto default_path = argc == 1 ? fs::current_path() : argv[1];

    if(!fs::exists(default_path)) {
        std::cerr << "Error: Path does not exist: " << default_path << std::endl;
        return 1;
    }


    // Launch scanner
    Scanner scanner = Scanner(default_path);
    std::thread scanner_thread([&scanner]() { scanner.scan(); });


    // Init shared menu & pie data
    int selected = 0;
    auto data = std::make_shared<AppData::BonsaiData>();
    
    data->menu_entries = std::make_shared<std::vector<AppData::BonsaiMenuEntry>>();
    data->menu_labels = std::make_shared<std::vector<std::string>>();
    data->path = std::make_shared<std::string>(default_path);


    /* Init menu:
    - Use a container to keep focus through the entire render.
      If a menu component is embeded in a slew of elements, it becomes static
    */
    MenuOption option;

    auto menu_component = BonsaiMenu::menu(&screen, data, &selected, default_path, option);
    auto menu_container = Container::Vertical({menu_component});

    std::thread menu_thread(BonsaiMenu::worker, &screen, data, &scanner, default_path);

    
    /* Init pie:
    - No need for a containter this time, as the component isn't interactive
    */
    data->pie_entries = std::make_shared<std::vector<AppData::BonsaiPieEntry>>();
    auto pie_component = BonsaiPie::pie(data);
    
    std::thread pie_thread(BonsaiPie::worker, &screen, data, &scanner, default_path);

    // Init main UI
    auto window = Renderer(menu_container, [&] {
        std::lock_guard<std::mutex> lock_m(data->menu_mutex);
        std::lock_guard<std::mutex> lock_p(data->pie_mutex);
    
        return hbox({
            vbox({
                text(" "),
                text("Explorer") | center | bold,
                separator(),
                hbox({
                    menu_component->Render() | size(WIDTH, EQUAL, config.SIDEBAR_WIDTH) | frame,
                    text(" ")
                })
            }),
            separator(),
            pie_component->Render()
        });
    });


    // Render UI
    screen.Loop(window);
    

    // Stop app
    AppData::stop(data);

    menu_thread.join();
    pie_thread.join();

    scanner.stop();
    scanner_thread.join();

    return 0;
}
