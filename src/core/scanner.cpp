#include "../../include/core/scanner.hpp"

#include <iostream>

std::unique_ptr<Scanner> Scanner::instance;
std::mutex Scanner::instance_mutex;

Scanner::Scanner(const fs::path& path)
    : root(path)
{
    running = true;
    worker = std::thread(&Scanner::run, this);
}

Scanner::~Scanner() {
    running = false;
    if (worker.joinable())
        worker.join();
}

void Scanner::scan(const fs::path& path) {
    std::lock_guard lock(instance_mutex);

    if (!instance)
        instance.reset(new Scanner(path));
}

uint64_t Scanner::get(const fs::path& path) {
    std::lock_guard lock(instance_mutex);
    if (!instance)
        return 0;

    std::lock_guard mapLock(instance->map_mutex);
    auto it = instance->dir_sizes.find(path.string());
    
    return (it != instance->dir_sizes.end()) ? it->second : 0;
}

void Scanner::run() {
    compute_dir_sizes(root);
    running = false;
}

bool Scanner::is_virtual_fs(const fs::path& path) {
    std::string p = path.string();
    return p.rfind("/proc", 0) == 0 ||
           p.rfind("/sys", 0)  == 0 ||
           p.rfind("/dev", 0)  == 0 ||
           p.rfind("/run", 0)  == 0 ||
           p.rfind("/tmp", 0)  == 0;
}

uint64_t Scanner::compute_dir_sizes(const fs::path& dir) {
    uint64_t total_size = 0;

    if (!fs::exists(dir) || !fs::is_directory(dir) || is_virtual_fs(dir))
        return 0;

    struct stat st{};
    if (lstat(dir.c_str(), &st) != 0)
        return 0;

    Inode inode{st.st_dev, st.st_ino};
    if (visited.count(inode))
        return 0;

    visited.insert(inode);

    for (auto& entry :
         fs::directory_iterator(dir, fs::directory_options::skip_permission_denied)) {
        try {
            if (fs::is_symlink(entry.path()))
                continue;

            if (fs::is_regular_file(entry.path())) {
                total_size += fs::file_size(entry);
            }
            else if (fs::is_directory(entry.path())) {
                total_size += compute_dir_sizes(entry.path());
            }
        }
        catch (...) {}
    }

    {
        std::lock_guard lock(map_mutex);
        dir_sizes[dir.string()] = total_size;
    }

    return total_size;
}
