#include "../include/core/scanner.hpp"

#include <ftxui/component/screen_interactive.hpp>

#include <iostream>
#include <thread>

#define DEFAULT_PATH "/"

int main() {
    using namespace ftxui;

    Scanner s = Scanner(DEFAULT_PATH);
    
    std::thread t([&s]() { s.scan(); });

    for(int i = 0; i < 100; i++) {
        std::cout << s.get("/") << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    t.join();

    return 0;
}
