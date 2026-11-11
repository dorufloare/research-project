// times the stencil for every layout and traversal, prints min and median
// usage: bench_stencil [K] [n]
#include <cstdlib>
#include <iostream>
#include <vector>

#include "layouts/row_major.hpp"
#include "layouts/morton.hpp"
#include "layouts/morton_fast.hpp"
#include "layouts/hilbert.hpp"
#include "layouts/tiled.hpp"
#include "kernels/stencil.hpp"
#include "timing.hpp"

const int ROUNDS = 7;

template <std::size_t K, typename Layout>
void add_cases(std::vector<Case>& cases, const std::string& name, const Layout& layout,
               std::uint32_t n, std::vector<double>& in, std::vector<double>& out,
               const std::vector<std::array<std::uint32_t, K>>& centers) {
    cases.push_back({name + ",coords", [&layout, &in, &out, n] {
        sweep_coords<K>(layout, in.data(), out.data(), n);
    }, {}});
    cases.push_back({name + ",random", [&layout, &in, &out, &centers] {
        sweep_random<K>(layout, in.data(), out.data(), centers);
    }, {}});
    cases.push_back({name + ",layout", [&layout, &in, &out, n] {
        sweep_layout<K>(layout, in.data(), out.data(), n);
    }, {}});
}

template <std::size_t K>
void run_all(std::uint32_t n) {
    RowMajor<K> row_major(n);
    Tiled<K> tiled(n);
    Morton<K> morton(n);
    MortonFast<K> morton_fast(n);
    Hilbert<K> hilbert(n);

    // all layouts have n^K cells, so they can use the same arrays
    std::vector<double> in(row_major.size(), 1.0);
    std::vector<double> out(row_major.size(), 0.0);

    // as many random points as inner points
    std::size_t count = 1;
    for (std::size_t i = 0; i < K; i++)
        count *= n - 2;
    auto centers = random_centers<K>(n, count, 1);

    std::vector<Case> cases;
    add_cases<K>(cases, "row_major", row_major, n, in, out, centers);
    add_cases<K>(cases, "tiled", tiled, n, in, out, centers);
    add_cases<K>(cases, "morton", morton, n, in, out, centers);
    add_cases<K>(cases, "morton_fast", morton_fast, n, in, out, centers);
    add_cases<K>(cases, "hilbert", hilbert, n, in, out, centers);
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

    std::cout << "layout,traversal,k,n,min_ms,median_ms\n";
    if (k == 2)
        run_all<2>(n);
    else if (k == 3)
        run_all<3>(n);
    else if (k == 4)
        run_all<4>(n);
    else
        std::cerr << "K has to be 2, 3 or 4\n";
}
