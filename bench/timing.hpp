#pragma once

#include <algorithm>
#include <chrono>
#include <functional>
#include <string>
#include <vector>

// runs f once, returns the time in ms
template <typename F>
double time_once(F f) {
    auto start = std::chrono::steady_clock::now();
    f();
    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

// runs f once to warm up, then 5 times, returns the median time in ms
template <typename F>
double time_it(F f) {
    f();
    std::vector<double> times;
    for (int i = 0; i < 5; i++)
        times.push_back(time_once(f));
    std::sort(times.begin(), times.end());
    return times[2];
}

struct Case {
    std::string name;
    std::function<void()> run;
    std::vector<double> times;
};

// each round runs every case once, so if the machine slows down it hits all cases
inline void run_in_turns(std::vector<Case>& cases, int rounds) {
    for (int r = 0; r < rounds; r++)
        for (auto& c : cases)
            c.times.push_back(time_once(c.run));
    for (auto& c : cases)
        std::sort(c.times.begin(), c.times.end());
}
