#pragma once

#include <algorithm>
#include <chrono>
#include <vector>

// runs f once to warm up, then 5 times, returns the median time in ms
template <typename F>
double time_it(F f) {
    f();
    std::vector<double> times;
    for (int i = 0; i < 5; i++) {
        auto start = std::chrono::steady_clock::now();
        f();
        auto end = std::chrono::steady_clock::now();
        times.push_back(std::chrono::duration<double, std::milli>(end - start).count());
    }
    std::sort(times.begin(), times.end());
    return times[2];
}
