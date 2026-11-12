#pragma once

#include <sys/prctl.h>

#include <fstream>
#include <string>

// wsl has transparent huge pages on "always", this turns them off so we get 4KB pages
inline void small_pages_only() {
    prctl(PR_SET_THP_DISABLE, 1, 0, 0, 0);
}

// how much of our memory the kernel really put in huge pages
inline std::string huge_pages_in_use() {
    std::ifstream f("/proc/self/smaps_rollup");
    std::string line;
    while (std::getline(f, line))
        if (line.rfind("AnonHugePages:", 0) == 0)
            return line;
    return "AnonHugePages: ?";
}
