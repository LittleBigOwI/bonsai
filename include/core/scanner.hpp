#pragma once

#include <filesystem>
#include <functional>
#include <mutex>
#include <map>

namespace fs = std::filesystem;

class Scanner {
public:
    Scanner(const fs::path& path);
    ~Scanner();

    void setCallback(std::function<void(const fs::path&, uintmax_t)> callback);
    void scan();

private:
    std::function<void(const fs::path&, uintmax_t)> folder_size_callback;

    std::mutex folder_sizes_mutex;
    std::map<std::string, uintmax_t> folder_sizes;

    bool is_done = false;

    void producer();
    void consumer();
};