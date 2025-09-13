#include "scanner.hpp"

#include "../config/config.hpp"

#include <unordered_map>
#include <sys/statfs.h>
#include <algorithm>
#include <iostream>

bool Scanner::isVirtualFs(const fs::path& path) {
    struct statfs sfs;
    if (statfs(path.c_str(), &sfs) != 0) return false;

    switch (sfs.f_type) {
        case 0x9fa0:       // devtmpfs
        case 0x62656572:   // debugfs
        case 0x53464846:   // shmfs / tmpfs
        case 0x858458f6:   // ramfs
        case 0x1cd1:       // aufs / overlayfs
        case 0x01021994:   // tmpfs
        case 0x19830326:   // cgroup
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
    std::shared_ptr<TreeNode> node;
    if (path == "/" && parent_node->cached_full_path == "/") {
        node = parent_node;
    } else {
        node = std::make_shared<TreeNode>();
        node->name = path.filename().string();
        node->is_dir = true;
        node->parent = parent_node;
        node->cached_full_path = (parent_node->cached_full_path == "/" ? "/" : parent_node->cached_full_path.string() + "/") + node->name;
        node->cached_full_path = fs::absolute(node->cached_full_path).lexically_normal();

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

int Scanner::deleteNode(const std::shared_ptr<TreeNode>& node) {
    const auto& path = node->cached_full_path;
    std::error_code ec;

    if (!fs::exists(path, ec)) {
        return ec.value() != 0 ? ec.value() : static_cast<int>(std::errc::no_such_file_or_directory);
    }

    if (fs::is_directory(path, ec)) {
        fs::remove_all(path, ec);
    } else {
        fs::remove(path, ec);
    }

    if (ec) {
        return ec.value();
    }

    deleteNodeRec(node, true);
    return 0;
}

void Scanner::deleteNodeRec(const std::shared_ptr<TreeNode>& node, bool top_level) {
    if (!node) return;

    uintmax_t total_size = 0;
    uintmax_t total_files = 0;
    uintmax_t total_folders = 0;

    {
        std::lock_guard<std::mutex> lock(node->node_mutex);
        total_size = node->size.load(std::memory_order_relaxed);
        total_files = node->files.load(std::memory_order_relaxed);
        total_folders = node->folders.load(std::memory_order_relaxed);
    }

    auto parent = node->parent.lock();
    parent->files.fetch_sub(node->is_dir ? 0 : 1, std::memory_order_relaxed);
    parent->folders.fetch_sub(node->is_dir ? 1 : 0, std::memory_order_relaxed);

    if (parent) {
        std::lock_guard<std::mutex> lock(parent->node_mutex);
        parent->children.erase(
            std::remove_if(parent->children.begin(), parent->children.end(), [&](const std::shared_ptr<TreeNode>& n){ return n == node; }),
            parent->children.end()
        );     
    }

    snapshot_->node_map.erase(node->cached_full_path);
    for (auto& child : node->children) {
        deleteNodeRec(child, false);
    }

    if(!top_level) return;

    auto current = parent;
    while(current) {
        current->size.fetch_sub(total_size, std::memory_order_relaxed);
        current = current->parent.lock();       
    }   

    snapshot_->total_size.fetch_sub(total_size, std::memory_order_relaxed);
    snapshot_->total_files.fetch_sub(total_files, std::memory_order_relaxed);
    snapshot_->total_dirs.fetch_sub(total_folders, std::memory_order_relaxed);
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
            node->cached_full_path = fs::absolute(node->cached_full_path).lexically_normal();

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
    const Config& config = Config::get();

    for (int i = 0; i < config.SCANNER_THREADS; ++i)
        workers.emplace_back(&Scanner::worker, this);

    for (auto& t : workers)
        t.join();
}

void Scanner::printSnapshot(const std::shared_ptr<TreeNode>& node, int depth) {
    if (!node || depth >= 5) return;
    
    std::string indent(depth * 2, ' ');
    std::cout << indent << (node->is_dir ? 
        "[DIR] " : "[FILE] ") << node->name << 
        " | size: " << node->size << 
        " | files: " << node->files << 
        " | folders: " << node->folders << "\n";
        
    for (auto& child : node->children) {
        printSnapshot(child, depth + 1);
    }
}

std::shared_ptr<TreeNode> Scanner::getNode(const std::string& path, ScanSnapshot& snapshot) {
    std::lock_guard<std::mutex> lock(snapshot.node_map_mutex);
    auto it = snapshot.node_map.find(path);
    if (it != snapshot.node_map.end()) return it->second;
    return nullptr;
}