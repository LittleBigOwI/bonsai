/* SCANNER CLASS
Explanation:
- This class recursively scans a filesystem path and computes the total sizes of directories.
- It avoids counting the same inode twice (to handle hard links) using the `visited` set.
- Thread-safe access to the `dir_sizes` map using a mutex.
- Virtual filesystems (like /proc, /sys) and network shares (NFS, SMB) are ignored.
- Public interface includes:
    - `scan()` to start scanning the root path.
    - `get(path)` to retrieve the total size of a directory (thread-safe).
*/

#pragma once

#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <mutex>

namespace fs = std::filesystem;

class Scanner {
private:
    fs::path path;

    std::unordered_map<std::string, uint64_t> dir_sizes;
    std::mutex map_mutex;

    /* Inode struct
       - Represents a filesystem inode (device + inode number)
       - Used to avoid double-counting hard links
       - Fully private and only visible inside Scanner
    */
    struct Inode {
        dev_t dev;  // Device ID
        ino_t ino;  // Inode number

        bool operator==(const Inode& other) const {
            return dev == other.dev && ino == other.ino;
        }
    };

    /* InodeHash struct
       - Hash function for unordered_set of Inode
       - Combines device ID and inode number
    */
    struct InodeHash {
        std::size_t operator()(const Inode& i) const {
            return std::hash<dev_t>()(i.dev) ^ std::hash<ino_t>()(i.ino);
        }
    };

    std::unordered_set<Inode, InodeHash> visited;
    
    bool isVirtualFs(const fs::path& path);

    /* computeDirSizes(dir)
       - Recursively computes the total size of a directory.
       - Adds sizes of files and subdirectories.
       - Skips symlinks, virtual filesystems, and already visited inodes.
       - Updates `dir_sizes` in a thread-safe manner.
    */
    uint64_t computeDirSizes(const fs::path& dir);

public:
    Scanner(const fs::path& _path) : path(_path) {};

    void scan();

    uint64_t get(const fs::path& path);
};