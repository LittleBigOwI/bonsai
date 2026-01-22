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

    void setCallback(std::function<void(const bool)> callback);
    void scan();

private:
    std::function<void(const bool)> folder_size_callback;

    std::mutex folder_sizes_mutex;
    std::map<std::string, uintmax_t> folder_sizes;

    fs::path& path;

    bool is_done = false;

    void producer();
    void consumer();
};