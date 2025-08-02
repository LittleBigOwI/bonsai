#pragma once

#include <thread>

class DataFormat {
public:
    static std::string toReadable(uintmax_t bytes);
};
