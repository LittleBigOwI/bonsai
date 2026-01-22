#include "../include/core/scanner.hpp"

#include <ftxui/component/screen_interactive.hpp>

#include <iostream>
#include <thread>

#define DEFAULT_PATH "/"

int main() {
    using namespace ftxui;

    Scanner::scan(DEFAULT_PATH);

    for (int i = 0; i < 10; i++) {
        uint64_t size = Scanner::get("/home");

        std::cout << "/home size: " << size << " bytes\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
