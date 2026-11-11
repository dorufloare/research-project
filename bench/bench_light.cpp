// light stencil with prefetch along the curve, distance 0 = no prefetch
// usage: bench_light [K] [n]   (n has to be a power of 2)
#include <cstdlib>
#include <iostream>
#include <vector>

#include "kernels/stencil_light.hpp"
#include "layouts/morton_fast.hpp"
#include "timing.hpp"

const int ROUNDS = 15;

template <std::size_t K>
void add_cases(std::vector<Case>& cases, const std::string& name,
               const std::array<std::uint64_t, K>& m, std::vector<double>& in,
               std::vector<double>& out) {
    for (std::size_t dist : {0, 64, 512, 4096, 32768})
        cases.push_back({name + "," + std::to_string(dist), [&m, &in, &out, dist] {
            sweep_light<K>(m, out.size(), in.data(), out.data(), dist);
        }, {}});
}

template <std::size_t K>
void run_all(std::uint32_t n) {
    MortonFast<K> morton(n);
    auto row_major = row_major_masks<K>(n);
    std::vector<double> in(morton.size() + PREFETCH_PAD, 1.0);
    std::vector<double> out(morton.size(), 0.0);

    std::vector<Case> cases;
    add_cases<K>(cases, "row_major", row_major, in, out);
    add_cases<K>(cases, "morton", morton.masks(), in, out);
    run_in_turns(cases, ROUNDS);

    for (const auto& c : cases)
        std::cout << c.name << "," << K << "," << n << "," << c.times[0] << ","
                  << c.times[ROUNDS / 2] << "\n";
    std::cerr << "check: " << out[out.size() / 2] << "\n";
}

int main(int argc, char** argv) {
    int k = argc > 1 ? std::atoi(argv[1]) : 3;
    int default_n = k == 2 ? 4096 : (k == 3 ? 256 : 64);
    std::uint32_t n = argc > 2 ? std::atoi(argv[2]) : default_n;

    std::cout << "layout,distance,k,n,min_ms,median_ms\n";
    if (k == 2)
        run_all<2>(n);
    else if (k == 3)
        run_all<3>(n);
    else if (k == 4)
        run_all<4>(n);
    else
        std::cerr << "K has to be 2, 3 or 4\n";
}
