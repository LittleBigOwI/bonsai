#pragma once

#include <filesystem>
#include <unordered_map>
#include <vector>
#include <string>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <queue>
#include <functional>
#include <memory>

namespace fs = std::filesystem;

#define SCANNER_THREADS 16

struct TreeNode {
    std::string name;
    uintmax_t size = 0;
    uintmax_t files = 0;
    uintmax_t folders = 0;
    bool is_dir = false;

    std::vector<std::shared_ptr<TreeNode>> children;
    std::weak_ptr<TreeNode> parent;

    std::string fullPath() const {
        if (auto p = parent.lock()) {
            if (p->parent.expired())
                return "/" + name;
            return p->fullPath() + "/" + name;
        }
        return name;
    }
};


struct ScanSnapshot {
    std::atomic<uintmax_t> total_size{0};
    std::atomic<uintmax_t> total_files{0};
    std::atomic<uintmax_t> total_dirs{0};

    std::mutex tree_mutex;
    std::shared_ptr<TreeNode> root;
};

class Scanner {
public:
    explicit Scanner(fs::path root, ScanSnapshot* snapshot) : root_(std::move(root)), snapshot_(snapshot) {
        snapshot_->root = std::make_shared<TreeNode>();
        snapshot_->root->name = "/";
        snapshot_->root->is_dir = true;
    }

    void scan();
    void setCallback(std::function<void()> callback);

    static std::shared_ptr<TreeNode> getNode(const std::string& path, ScanSnapshot& snapshot);

private:
    struct DirTask {
        fs::path path;
        std::shared_ptr<TreeNode> node;
    };

    fs::path root_;
    ScanSnapshot* snapshot_ = nullptr;
    std::function<void()> update_callback_;

    std::queue<DirTask> task_queue_;
    std::mutex queue_mutex_;
    std::condition_variable cv_;
    std::atomic<int> active_tasks_{0};

    void enqueue(const fs::path& path, std::shared_ptr<TreeNode> parent_node);
    void worker();
    bool isVirtualFs(const fs::path& path);
};
