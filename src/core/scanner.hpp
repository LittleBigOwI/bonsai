#pragma once

#include <condition_variable>
#include <unordered_map>
#include <filesystem>
#include <functional>
#include <atomic>
#include <thread>
#include <queue>

namespace fs = std::filesystem;

#define SCANNER_THREADS 16

struct TreeNode {
    std::string name;
    std::atomic<uintmax_t> size{0};
    std::atomic<uintmax_t> files{0};
    std::atomic<uintmax_t> folders{0};
    bool is_dir = false;

    std::vector<std::shared_ptr<TreeNode>> children;
    std::weak_ptr<TreeNode> parent;

    std::mutex node_mutex;
    fs::path cached_full_path;
};

struct ScanSnapshot {
    std::atomic<uintmax_t> total_size{0};
    std::atomic<uintmax_t> total_files{0};
    std::atomic<uintmax_t> total_dirs{0};

    std::shared_ptr<TreeNode> root;

    std::unordered_map<std::string, std::shared_ptr<TreeNode>> node_map;
    std::mutex node_map_mutex;
};

class Scanner {
public:
    explicit Scanner(fs::path root, ScanSnapshot* snapshot): root_(std::move(root)), snapshot_(snapshot) {
        snapshot_->root = std::make_shared<TreeNode>();
        snapshot_->root->name = "/";
        snapshot_->root->is_dir = true;
        snapshot_->root->cached_full_path = "/";

        {
            std::lock_guard<std::mutex> lock(snapshot_->node_map_mutex);
            snapshot_->node_map["/"] = snapshot_->root;
        }
    }

    void scan();
    void setCallback(std::function<void()> callback) { update_callback_ = std::move(callback); }

    static std::shared_ptr<TreeNode> getNode(const std::string& path, ScanSnapshot& snapshot);
    static void printSnapshot(const std::shared_ptr<TreeNode>& node, int depth = 0);

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
    fs::path getFullPath(const std::shared_ptr<TreeNode>& node);
};