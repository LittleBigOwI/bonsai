#include "widgets/canvas_ext.hpp"
#include "interface/piechart.hpp"
#include "interface/sidebar.hpp"
#include "utils/dataformat.hpp"
#include "core/scanner.hpp"

#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#define SIDEBAR_WIDTH 50
#define SIDEBAR_PATH "/"

int main() {
    using namespace ftxui;

    ScreenInteractive screen = ScreenInteractive::Fullscreen();
    ScanSnapshot snapshot;

    auto sidebar = ui::RenderSidebar(SIDEBAR_PATH, SIDEBAR_WIDTH, &snapshot);

    // Pie chart UI (live updating)
    // auto piechart = Renderer([&] {
    //     std::lock_guard<std::mutex> lock(snapshot_mutex);

    //     // Build piechart ring from current snapshot
    //     ui::PieRing ring = ui::BuildPieRingFromSnapshot(snapshot);  // <- You implement this
    //     std::string center_label = DataFormat::toReadable(snapshot.total_size.load());

    //     return ui::RenderPieChart(ring, center_label);
    // });

    auto ui = Container::Horizontal({
        sidebar,
        // piechart,
    });

    std::thread scan_thread([&] {
        Scanner scanner(SIDEBAR_PATH, &snapshot);
        scanner.setCallback([&screen]() {
            screen.PostEvent(Event::Custom);
        });

        scanner.scan(); 
        screen.PostEvent(Event::Custom);
    });
    
    screen.Loop(ui);

    scan_thread.join();
    return 0;
}

// int main() {
//     using namespace ftxui;

//     auto screen = ScreenInteractive::Fullscreen();

//     ui::PieRing ring = {
//         {
//             {
//                 "home 50%", 40.0, {
//                     {
//                         "pictures 30%", 30.0, {
//                             {"phone 30%", 30.0}
//                         }
//                     },
//                     {"videos 10%", 10.0}
//                 }
//             },
//             {
//                 "usr 30%", 30.0, {
//                     {"lib 15%", 15.0},
//                 }
//             },
//             {
//                 "bin 20%", 20.0, {
//                     {
//                         "librewolf 10%", 10.0, {
//                             {"internal 10%", 10.0}
//                         }
//                     },
//                 }
//             },
//             {
//                 "var 10%", 10.0
//             }
//         }
//     };
//     std::string center_label = "140.29\nGiB";
//     auto right_pane = ui::RenderPieChart(ring, center_label);

//     ScanSnapshot snapshot;
//     std::mutex render_mutex;
    
//     auto sidebar = ui::RenderSidebar(SIDEBAR_PATH, SIDEBAR_WIDTH);
//     auto app = Container::Horizontal({
//         sidebar,
//         right_pane,
//     });

//     screen.Loop(app);
// }

// int main() {
//     using namespace ftxui;

//     ScreenInteractive screen = ScreenInteractive::TerminalOutput();

//     ScanSnapshot snapshot;
//     std::mutex render_mutex;

//     std::string label = "Initializing...";

//     auto ui = Renderer([&] {
//         std::lock_guard<std::mutex> lock(render_mutex);

//         uintmax_t size_mb = snapshot.total_size / (1024 * 1024);
//         uintmax_t files = snapshot.total_files;
//         uintmax_t dirs = snapshot.total_dirs;

//         std::string stat = "Scanned: " + std::to_string(size_mb) + " MB / " +
//                            std::to_string(files) + " files / " +
//                            std::to_string(dirs) + " folders " +
//                            "/home: " + DataFormat::toReadable(Scanner::getStats("/home/littlebigowl", snapshot).size);

//         return vbox({
//             text("Disk Scanner Progress") | bold,
//             separator(),
//             text(stat),
//         }) | border;
//     });

//     // Scanner thread
//     std::thread scan_thread([&] {
//         Scanner scanner("/", snapshot);
//         scanner.scan();

//         screen.PostEvent(Event::Custom); // final update
//         screen.PostEvent(Event::Escape); // exit UI
//     });

//     // Periodic UI refresh
//     std::thread refresher([&] {
//         while (true) {
//             std::this_thread::sleep_for(std::chrono::milliseconds(200));
//             screen.PostEvent(Event::Custom);
//         }
//     });

//     screen.Loop(ui);
//     scan_thread.join();
//     refresher.detach(); // or handle gracefully

//     return 0;
// }