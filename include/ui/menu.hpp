/* UPDATED MENU COMPONENT FOR BONSAI
New menu structure: no class inheriting, just using base ftxui components (more maintainable)
This version of the file is a standalone version (no piechart, but bull contents)
Note that directory sizes aren't computed (scanner's job)

- The menu is initialized with empty entries
- Once the menu is created by the main thread, a worker (separate thread) is created
- This worker has a given path and adds the contents of that path into BonsaiMenuData, then sleep on a condition variable
- BonsaiMenuData contains file size for each entry (0 for dirs), full path and label
  We have to have separate vectors entries and lables because the menu component only takes in a vector of strings
  These two vector are updated together, always
  When the menu re-renders, it will read from labels and entries to get labels and file info
- As the menu takes in BonsaiMenuData, it is updated as the worker updates it
- The user is free to scroll up and down the menu of files in the current dir, and view file sizes.
- When the user selects a dir with enter key, the condition variable wakes up with an updated path, the one of the selected entry aquired via entries
- The worker goes for one iteration and sleeps until woken up again.
- The boolean path changed is only there in case the thread accidentally wakes up.
*/

#pragma once

#include "../../include/core/scanner.hpp"

#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <filesystem>

using namespace ftxui;

namespace fs = std::filesystem;

class BonsaiMenu {
public:
    /* BonsaiMenuEntry
    - Represents a single filesystem entry in the menu.
    - Contains:
      size   → folder size (0 for directories)
      label  → display name in the menu
      path   → absolute filesystem path
      is_dir → whether entry is a directory
    */
    struct BonsaiMenuEntry {
        std::uintmax_t size;
        std::string label;
        std::string path;
        
        bool is_dir;
    };

    /* BonsaiMenuData
    - Shared state between UI and worker thread.
    - entries_mutex protects entries/labels updates.
    - cv_mutex + condition variable coordinate worker sleeping/waking.
    - path_changed signals directory navigation.
    - stop signals worker termination.
    */
    struct BonsaiMenuData {
        std::shared_ptr<std::vector<BonsaiMenuEntry>> entries;
        std::mutex entries_mutex;

        std::shared_ptr<std::vector<std::string>> labels;
        std::shared_ptr<std::string> path;

        std::condition_variable cv;
        std::mutex cv_mutex;

        bool path_changed = false;
        bool stop = false;
    };

    static Component menu(ScreenInteractive* screen, std::shared_ptr<BonsaiMenuData> data, int* selected, const fs::path& default_path, MenuOption options);
    static void worker(ScreenInteractive* screen, std::shared_ptr<BonsaiMenuData> data, Scanner* scanner, const fs::path& default_path);
    
    /* stop()
    - Requests worker thread shutdown.
    - Sets stop flag and wakes condition variable.
    - Intended to be called during application exit.
    */
    static void stop(std::shared_ptr<BonsaiMenuData> data);
};