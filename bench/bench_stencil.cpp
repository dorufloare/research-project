// times the stencil for every layout and traversal
// usage: bench_stencil [K] [n]
#include <array>
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

double check = 0;  // we use the results so the compiler can't remove the loops

template <std::size_t K, typename Layout>
void run(const char* name, const Layout& layout, std::uint32_t n,
         const std::vector<std::array<std::uint32_t, K>>& centers) {
    std::vector<double> in(layout.size(), 1.0);
    std::vector<double> out(layout.size(), 0.0);
    const char* traversals[] = {"coords", "random", "layout"};

    for (int t = 0; t < 3; t++) {
        double ms;
        if (t == 0)
            ms = time_it([&] { sweep_coords<K>(layout, in.data(), out.data(), n); });
        else if (t == 1)
            ms = time_it([&] { sweep_random<K>(layout, in.data(), out.data(), centers); });
        else
            ms = time_it([&] { sweep_layout<K>(layout, in.data(), out.data(), n); });

        for (double v : out)
            check += v;
        std::cout << name << "," << traversals[t] << "," << K << "," << n << "," << ms << "\n";
    }
}

template <std::size_t K>
void run_all(std::uint32_t n) {
    // as many random points as inner points
    std::size_t count = 1;
    for (std::size_t i = 0; i < K; i++)
        count *= n - 2;
    auto centers = random_centers<K>(n, count, 1);

    run<K>("row_major", RowMajor<K>(n), n, centers);
    run<K>("tiled", Tiled<K>(n), n, centers);
    run<K>("morton", Morton<K>(n), n, centers);
    run<K>("morton_fast", MortonFast<K>(n), n, centers);
    run<K>("hilbert", Hilbert<K>(n), n, centers);
}

int main(int argc, char** argv) {
    int k = argc > 1 ? std::atoi(argv[1]) : 3;
    int default_n = k == 2 ? 4096 : (k == 3 ? 256 : 64);
    std::uint32_t n = argc > 2 ? std::atoi(argv[2]) : default_n;

    std::cout << "layout,traversal,k,n,ms\n";
    if (k == 2)
        run_all<2>(n);
    else if (k == 3)
        run_all<3>(n);
    else if (k == 4)
        run_all<4>(n);
    else
        std::cerr << "K has to be 2, 3 or 4\n";

    std::cerr << "check: " << check << "\n";
}
