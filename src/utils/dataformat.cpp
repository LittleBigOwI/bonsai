#include "dataformat.hpp"

#include <iomanip>

std::string DataFormat::toReadable(uintmax_t bytes) {
    return toReadable(bytes, "");
}

std::string DataFormat::toReadable(uintmax_t bytes, const std::string& spacer) {
    constexpr uintmax_t KB = 1024;
    constexpr uintmax_t MB = KB * 1024;
    constexpr uintmax_t GB = MB * 1024;

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);

    if (bytes >= GB)
        oss << (double)bytes / GB << spacer << "GiB";
    else if (bytes >= MB)
        oss << (double)bytes / MB << spacer << "MiB";
    else if (bytes >= KB)
        oss << (double)bytes / KB << spacer << "KiB";
    else
        oss << bytes << spacer << "B";

    return oss.str();
}