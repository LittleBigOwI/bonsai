#pragma once

#include <filesystem>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>
#include <sys/stat.h>

namespace fs = std::filesystem;

class Scanner {
public:
    static void scan(const fs::path& path);
    static uint64_t get(const fs::path& path);

    ~Scanner();

private:
    Scanner(const fs::path& path);
    Scanner(const Scanner&) = delete;
    Scanner& operator=(const Scanner&) = delete;

    void run();
    uint64_t compute_dir_sizes(const fs::path& dir);

    static bool is_virtual_fs(const fs::path& path);

    struct Inode {
        dev_t dev;
        ino_t ino;

        bool operator==(const Inode& other) const {
            return dev == other.dev && ino == other.ino;
        }
    };

    struct InodeHash {
        std::size_t operator()(const Inode& i) const {
            return std::hash<dev_t>()(i.dev)
                 ^ std::hash<ino_t>()(i.ino);
        }
    };

    fs::path root;

    std::unordered_map<std::string, uint64_t> dir_sizes;
    std::unordered_set<Inode, InodeHash> visited;

    std::mutex map_mutex;
    std::thread worker;
    std::atomic<bool> running{false};

    static std::unique_ptr<Scanner> instance;
    static std::mutex instance_mutex;
};
