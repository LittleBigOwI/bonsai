#pragma once

#include <filesystem>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <queue>
#include <functional>

namespace fs = std::filesystem;

#define SCANNER_THREADS 16

class Scanner {
public:
    explicit Scanner(fs::path root = "/");

    void scan();
    const std::unordered_map<std::string, uintmax_t>& getSizes() const;
    uintmax_t getTotalSize() const;
    uintmax_t getFileCount() const;

private:
    struct DirTask {
        fs::path path;
    };

    fs::path root_;

    std::queue<DirTask> task_queue_;
    std::mutex queue_mutex_;
    std::condition_variable cv_;
    std::atomic<int> active_tasks_{0};

    std::unordered_map<std::string, uintmax_t> dir_sizes_;
    std::unordered_map<std::string, std::vector<std::string>> dir_children_;
    std::mutex result_mutex_;

    std::atomic<uintmax_t> file_count_{0};
    std::atomic<uintmax_t> total_size_{0};

    void enqueue(const fs::path& path);
    void worker();
    void propagate(const std::string& root_path);
    bool isVirtualFs(const fs::path& path);
};
