#include "scanner.hpp"

#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/event.hpp>

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

void Scanner::enqueue(const fs::path& path) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    task_queue_.push({ path });
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
        uintmax_t local_size = 0;
        uintmax_t local_files = 0;
        uintmax_t local_folders = 0;
        std::vector<std::string> children;

        for (auto& entry : fs::directory_iterator(dir, fs::directory_options::skip_permission_denied)) {
            try {
                if (fs::is_symlink(entry)) continue;

                if (fs::is_regular_file(entry)) {
                    uintmax_t file_size = fs::file_size(entry);
                    local_size += file_size;
                    local_files++;
                    snapshot_->total_size += file_size;
                    snapshot_->total_files++;
                } else if (fs::is_directory(entry) && !isVirtualFs(entry)) {
                    std::string child_str = entry.path().string();
                    children.push_back(child_str);
                    local_folders++;
                    snapshot_->total_dirs++;
                    enqueue(entry.path());
                }
            } catch (...) {
                continue;
            }
        }

        {
            std::lock_guard<std::mutex> lock(snapshot_->map_mutex);
            snapshot_->path_stats[dir.string()] = {
                .size = local_size,
                .files = local_files,
                .folders = local_folders
            };
        }

        {
            std::lock_guard<std::mutex> lock(result_mutex_);
            dir_sizes_[dir.string()] = local_size;
            dir_children_[dir.string()] = children;
        }

        active_tasks_--;
        cv_.notify_all();
    }
}

void Scanner::propagate(const std::string& root_path) {
    this->update_callback_();
    std::unordered_set<std::string> visited;

    std::function<uintmax_t(const std::string&)> dfs = [&](const std::string& path) -> uintmax_t {
        if (visited.count(path)) return 0;
        visited.insert(path);

        uintmax_t total = 0;

        {
            std::lock_guard<std::mutex> lock(result_mutex_);
            total = dir_sizes_[path];
        }

        for (const auto& child : dir_children_[path]) {
            total += dfs(child);
        }

        {
            std::lock_guard<std::mutex> lock(result_mutex_);
            dir_sizes_[path] = total;
        }

        {
            std::lock_guard<std::mutex> lock(snapshot_->map_mutex);
            auto& stats = snapshot_->path_stats[path];
            stats.size = total;
        }

        return total;
    };

    dfs(root_path);
}


void Scanner::scan() {
    enqueue(root_);

    std::vector<std::thread> workers;
    for (int i = 0; i < SCANNER_THREADS; ++i)
        workers.emplace_back(&Scanner::worker, this);

    for (auto& t : workers)
        t.join();

    propagate(root_.string());
}

const std::unordered_map<std::string, uintmax_t>& Scanner::getSizes() const {
    return dir_sizes_;
}

uintmax_t Scanner::getTotalSize() const {
    return total_size_;
}

uintmax_t Scanner::getFileCount() const {
    return file_count_;
}

DirStats Scanner::getStats(const std::string& path, ScanSnapshot& snapshot) {
    std::lock_guard<std::mutex> lock(snapshot.map_mutex);
    auto it = snapshot.path_stats.find(path);
    if (it != snapshot.path_stats.end()) {
        return it->second;
    }
    return {};
}