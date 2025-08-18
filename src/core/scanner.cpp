#include "scanner.hpp"
#include <sys/statfs.h>
#include <algorithm>
#include <unordered_map>

bool Scanner::isVirtualFs(const fs::path& path) {
    struct statfs sfs;
    if (statfs(path.c_str(), &sfs) != 0) return false;

    switch (sfs.f_type) {
        // Common virtual filesystems on Linux
        case 0x9fa0:       // devtmpfs
        case 0x62656572:   // debugfs
        case 0x53464846:   // shmfs / tmpfs
        case 0x858458f6:   // ramfs
        case 0x1cd1:       // aufs / overlayfs
        case 0x01021994:   // tmpfs
        case 0x19830326:   // cgroup
        case 0x58465342:   // xfs
        case 0x65735546:   // fuse
            return true;
        default:
            return false;
    }
}

fs::path Scanner::getFullPath(const std::shared_ptr<TreeNode>& node) {
    if (!node->parent.lock()) return "/";
    return node->cached_full_path;
}

void Scanner::enqueue(const fs::path& path, std::shared_ptr<TreeNode> parent_node) {
    // If path is "/" we use the parent_node itself
    std::shared_ptr<TreeNode> node;
    if (path == "/" && parent_node->cached_full_path == "/") {
        node = parent_node; // Use root node directly
    } else {
        node = std::make_shared<TreeNode>();
        node->name = path.filename().string();
        node->is_dir = true;
        node->parent = parent_node;
        node->cached_full_path = (parent_node->cached_full_path == "/" ? "/" : parent_node->cached_full_path.string() + "/") + node->name;

        {
            std::lock_guard<std::mutex> lock(parent_node->node_mutex);
            parent_node->children.push_back(node);
        }

        {
            std::lock_guard<std::mutex> lock(snapshot_->node_map_mutex);
            snapshot_->node_map[node->cached_full_path] = node;
        }
    }

    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        task_queue_.push({ path, node });
        active_tasks_++;
        cv_.notify_one();
    }
}

void Scanner::worker() {
    while (true) {
        DirTask task;

        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            cv_.wait(lock, [this] { return !task_queue_.empty() || active_tasks_ == 0; });

            if (task_queue_.empty()) {
                if (active_tasks_ == 0) return;
                continue;
            }

            task = task_queue_.front();
            task_queue_.pop();
        }

        fs::path dir = task.path;
        auto node = task.node;

        uintmax_t local_size = 0;
        uintmax_t local_files = 0;
        uintmax_t local_folders = 0;

        for (auto& entry : fs::directory_iterator(dir, fs::directory_options::skip_permission_denied)) {
            try {
                if (fs::is_symlink(entry)) continue;

                if (fs::is_regular_file(entry)) {
                    uintmax_t file_size = fs::file_size(entry);

                    auto child = std::make_shared<TreeNode>();
                    child->name = entry.path().filename().string();
                    child->size.store(file_size, std::memory_order_relaxed);
                    child->files.store(1, std::memory_order_relaxed);
                    child->folders.store(0, std::memory_order_relaxed);
                    child->is_dir = false;
                    child->parent = node;
                    child->cached_full_path = node->cached_full_path.string() + "/" + child->name;

                    {
                        std::lock_guard<std::mutex> lock(node->node_mutex);
                        node->children.push_back(child);
                    }

                    {
                        std::lock_guard<std::mutex> lock(snapshot_->node_map_mutex);
                        snapshot_->node_map[child->cached_full_path] = child;
                    }

                    local_size += file_size;
                    local_files++;
                    snapshot_->total_size += file_size;
                    snapshot_->total_files++;
                } else if (fs::is_directory(entry) && !isVirtualFs(entry)) {
                    local_folders++;
                    snapshot_->total_dirs++;
                    enqueue(entry.path(), node);
                }
            } catch (...) {
                continue;
            }
        }

        node->size.fetch_add(local_size, std::memory_order_relaxed);
        node->files.fetch_add(local_files, std::memory_order_relaxed);
        node->folders.fetch_add(local_folders, std::memory_order_relaxed);

        // Propagate size up to parents atomically
        auto parent = node->parent.lock();
        while (parent) {
            parent->size.fetch_add(local_size, std::memory_order_relaxed);
            parent = parent->parent.lock();
        }

        if (update_callback_) update_callback_();

        active_tasks_--;
        cv_.notify_all();
    }
}

void Scanner::scan() {
    fs::path path = root_;
    std::vector<std::string> ancestors;

    while (path.has_parent_path()) {
        ancestors.push_back(path.filename().string());
        path = path.parent_path();
        if (path == "/") break;
    }
    std::reverse(ancestors.begin(), ancestors.end());

    std::shared_ptr<TreeNode> parent = snapshot_->root;
    parent->cached_full_path = "/";

    for (size_t i = 0; i < ancestors.size(); ++i) {
        const auto& name = ancestors[i];

        if (i == ancestors.size() - 1 && name == root_.filename().string()) break;

        auto it = std::find_if(parent->children.begin(), parent->children.end(),
                               [&](const std::shared_ptr<TreeNode>& n){ return n->name == name; });
        if (it != parent->children.end()) {
            parent = *it;
        } else {
            auto node = std::make_shared<TreeNode>();
            node->name = name;
            node->is_dir = true;
            node->parent = parent;
            node->cached_full_path = parent->cached_full_path.string() + "/" + name;

            {
                std::lock_guard<std::mutex> lock(parent->node_mutex);
                parent->children.push_back(node);
            }

            {
                std::lock_guard<std::mutex> lock(snapshot_->node_map_mutex);
                snapshot_->node_map[node->cached_full_path] = node;
            }

            parent = node;
        }
    }

    if (root_ == "/") {
        enqueue(root_, snapshot_->root);
    } else {
        enqueue(root_, parent);
    }

    std::vector<std::thread> workers;
    for (int i = 0; i < SCANNER_THREADS; ++i)
        workers.emplace_back(&Scanner::worker, this);

    for (auto& t : workers)
        t.join();

    // if(this->root_ == "/") {
    //     this->snapshot_->root = this->snapshot_->root->children[0];
    //     this->snapshot_->root->cached_full_path = "/";
    //     this->snapshot_->root->is_dir = true;
    //     this->snapshot_->root->name = "/";
    // }
}

std::shared_ptr<TreeNode> Scanner::getNode(const std::string& path, ScanSnapshot& snapshot) {
    std::lock_guard<std::mutex> lock(snapshot.node_map_mutex);
    auto it = snapshot.node_map.find(path);
    if (it != snapshot.node_map.end()) return it->second;
    return nullptr;
}