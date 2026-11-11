// how fast this machine can read / copy a big array, to compare with the stencil
// usage: bench_bandwidth [MB]
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "timing.hpp"

int main(int argc, char** argv) {
    std::size_t mb = argc > 1 ? std::atoi(argv[1]) : 128;
    std::size_t count = mb * 1024 * 1024 / sizeof(double);
    std::vector<double> in(count, 1.0);
    std::vector<double> out(count, 0.0);
    double check = 0;

    // 4 sums so the adds don't have to wait for each other
    double read_ms = time_it([&] {
        double s0 = 0, s1 = 0, s2 = 0, s3 = 0;
        for (std::size_t i = 0; i + 3 < count; i += 4) {
            s0 += in[i];
            s1 += in[i + 1];
            s2 += in[i + 2];
            s3 += in[i + 3];
        }
        check += s0 + s1 + s2 + s3;
    });

    double copy_ms = time_it([&] { std::copy(in.begin(), in.end(), out.begin()); });
    check += out[count / 2];

    double bytes = count * sizeof(double);
    std::cout << "test,mb,ms,gb_per_s\n";
    std::cout << "read," << mb << "," << read_ms << "," << bytes / read_ms / 1e6 << "\n";
    std::cout << "copy," << mb << "," << copy_ms << "," << 2 * bytes / copy_ms / 1e6 << "\n";
    std::cerr << "check: " << check << "\n";
}
