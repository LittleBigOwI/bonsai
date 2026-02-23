#include "../include/config/config.hpp"
#include "../include/core/app_data.hpp"
#include "../include/core/scanner.hpp"
#include "../include/ui/piechart.hpp"
#include "../include/ui/menu.hpp"

#include <ftxui/component/screen_interactive.hpp>

#include <iostream>
#include <thread>

namespace fs = std::filesystem;

/* TODO: bug? (can't reproduce anymore...)
-> scanner isn't finished.
-> menu scans for a folder, and updates sizes in list
-> this updates render, that updates piechart.
-> piechart worker doesn't wake up
-> for some reason percentages are updated but folder list isn't?

-> Maybe fixable with callback on scan completed (update render on complete) since this only happens when scanner is done
*/
int main(int argc, char* argv[]) {
    using namespace ftxui;

    auto config = Config::get();
    auto screen = ScreenInteractive::Fullscreen();
    auto default_path = argc == 1 ? fs::current_path() : fs::weakly_canonical(argv[1]);

    if(!fs::exists(default_path)) {
        std::cerr << "Error: Path does not exist: " << default_path << std::endl;
        return 1;
    }

    if(!fs::is_directory(default_path)) {
        std::cerr << "Error: Path isn't a directory: " << default_path << std::endl;
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
    data->selected = std::make_shared<int>(0);


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
    auto pie_component = BonsaiPie::pie(data, &scanner, default_path);

    pie_component->Render();
    
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

    // Quit with q
    auto app = CatchEvent(window, [&](Event event) {
        if (event == Event::Character('q') || event == Event::Character('Q')) {
            screen.Exit();
            return true;
        }
        return false;
    });

    // Render UI
    screen.Loop(app);
    

    // Stop app
    scanner.stop();
    AppData::stop(data);

    scanner_thread.join();
    menu_thread.join();
    pie_thread.join();

    return 0;
}