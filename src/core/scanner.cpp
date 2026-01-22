#include "../../include/core/scanner.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <cstdlib>
#include <filesystem>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>


void Scanner::producer() {
    // Run du command from coreutils
    std::string cmd = "du -k " + this->path.string() + " 2>/dev/null";

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        std::cerr << "Failed to run du\n";
        this->is_done = true;
        return;
    }

    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        std::string line(buffer);
        std::istringstream iss(line);
        uint64_t size;
        std::string path;
        if (iss >> size >> path) {
            std::lock_guard<std::mutex> lock(this->folder_sizes_mutex);
            this->folder_sizes[path] = size * 1024; // KB -> bytes
        }
    }

    pclose(pipe);
    this->is_done = true;
}

void Scanner::consumer() {

    while (!this->is_done) {
        {
            std::lock_guard<std::mutex> lock(this->folder_sizes_mutex);

            if(this->folder_size_callback) {
                this->folder_size_callback(false);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if(this->folder_size_callback) {
        this->folder_size_callback(true);
    }
}

void Scanner::scan() {
    std::thread prod(producer);
    std::thread cons(consumer);

    prod.join();
    cons.join();
}