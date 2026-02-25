/* MENU
New menu structure: no class inheriting, just using base ftxui components (more maintainable)
This version of the file is a standalone version (no piechart, but full contents)
Note that directory sizes aren't computed (scanner's job)

- The menu is initialized with empty entries
- Once the menu is created by the main thread, a worker (separate thread) is created
- This worker has a given path and adds the contents of that path into BonsaiMenuData, then sleep on a condition variable
- BonsaiMenuData contains file size for each entry (0 for dirs), full path and label
    - We have to have separate vectors entries and lables because the menu component only takes in a vector of strings
    - These two vector are updated together, always
    - When the menu re-renders, it will read from labels and entries to get labels and file info
- As the menu takes in BonsaiMenuData, it is updated as the worker updates it
- The user is free to scroll up and down the menu of files in the current dir, and view file sizes.
- When the user selects a dir with enter key, the condition variable wakes up with an updated path, the one of the selected entry aquired via entries
- The worker goes for one iteration and sleeps until woken up again.
- The boolean path changed is only there in case the thread accidentally wakes up.
*/

#pragma once

#include "../../include/core/app_data.hpp"
#include "../../include/core/scanner.hpp"

#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <filesystem>

using namespace ftxui;

namespace fs = std::filesystem;

class BonsaiMenu {
public:
    static Component menu(ScreenInteractive* screen, std::shared_ptr<AppData::BonsaiData> data, int* selected, const fs::path& default_path, MenuOption options);
    static void worker(ScreenInteractive* screen, std::shared_ptr<AppData::BonsaiData> data, Scanner* scanner, const fs::path& default_path);
};