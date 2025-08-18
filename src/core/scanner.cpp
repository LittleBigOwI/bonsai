#include "scanner.hpp"
#include <sys/statfs.h>

bool Scanner::isVirtualFs(const fs::path& path) {
    struct statfs sfs;
    if (statfs(path.c_str(), &sfs) != 0) return false;
    switch (sfs.f_type) {
        case 0x9fa0: case 0x62656572: case 0x01021994: case 0x1cd1:
        case 0x53464846: case 0x858458f6: case 0x19830326:
            return true;
        default:
            return false;
    }
}

void Scanner::enqueue(const fs::path& path, std::shared_ptr<TreeNode> parent_node) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    auto node = std::make_shared<TreeNode>();
    node->name = path.filename().string();
    node->is_dir = true;
    node->parent = parent_node;

    {
        std::lock_guard<std::mutex> tree_lock(snapshot_->tree_mutex);
        parent_node->children.push_back(node);
    }

    task_queue_.push({ path, node });
    active_tasks_++;
    cv_.notify_one();
}

void Scanner::setCallback(std::function<void()> callback) {
    update_callback_ = std::move(callback);
}

void Scanner::worker() {
    while (true) {
        DirTask task;

        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            cv_.wait(lock, [this] {
                return !task_queue_.empty() || active_tasks_ == 0;
            });

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
                    child->size = file_size;
                    child->files = 1;
                    child->folders = 0;
                    child->is_dir = false;
                    child->parent = node;

                    {
                        std::lock_guard<std::mutex> tree_lock(snapshot_->tree_mutex);
                        node->children.push_back(child);
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

        node->size += local_size;
        node->files += local_files;
        node->folders += local_folders;

        auto parent = node->parent.lock();
        while (parent) {
            parent->size += local_size;
            
            // Files & folders are local only
            // parent->files += local_files;
            // parent->folders += local_folders;
            
            parent = parent->parent.lock();
        }

        if (update_callback_) update_callback_();

        active_tasks_--;
        cv_.notify_all();
    }
}

void Scanner::scan() {
    enqueue(root_, snapshot_->root);

    std::vector<std::thread> workers;
    for (int i = 0; i < SCANNER_THREADS; ++i)
        workers.emplace_back(&Scanner::worker, this);

    for (auto& t : workers)
        t.join();
}

std::shared_ptr<TreeNode> Scanner::getNode(const std::string& path, ScanSnapshot& snapshot) {
    std::lock_guard<std::mutex> lock(snapshot.tree_mutex);
    std::function<std::shared_ptr<TreeNode>(std::shared_ptr<TreeNode>)> dfs;
    dfs = [&](std::shared_ptr<TreeNode> node) -> std::shared_ptr<TreeNode> {
        if (!node) return nullptr;
        if (node->name == path) return node;
        for (auto& child : node->children) {
            if (auto res = dfs(child)) return res;
        }
        return nullptr;
    };
    return dfs(snapshot.root);
}
