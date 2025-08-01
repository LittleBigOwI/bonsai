#include "scanner.hpp"
#include <sys/statfs.h>

Scanner::Scanner(fs::path root) : root_(std::move(root)) {}

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
        std::vector<std::string> children;

        for (auto& entry : fs::directory_iterator(dir, fs::directory_options::skip_permission_denied)) {
            try {
                if (fs::is_symlink(entry)) continue;

                if (fs::is_regular_file(entry)) {
                    uintmax_t file_size = fs::file_size(entry);
                    local_size += file_size;
                    total_size_ += file_size;
                    file_count_++;
                } else if (fs::is_directory(entry) && !isVirtualFs(entry)) {
                    std::string child_str = entry.path().string();
                    children.push_back(child_str);
                    enqueue(entry.path());
                }
            } catch (...) {
                continue;
            }
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