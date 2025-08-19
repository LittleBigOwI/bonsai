#pragma once

#include <thread>

class DataFormat {
    public:
        static std::string toReadable(uintmax_t bytes);
        static std::string toReadable(uintmax_t bytes, const std::string& spacer);
};
