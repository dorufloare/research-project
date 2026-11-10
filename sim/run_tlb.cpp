// runs the stencil traversals for every layout and counts TLB misses
// only the reads of the input array are counted
// usage: run_tlb [K] [n]
#include <array>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "layouts/row_major.hpp"
#include "layouts/morton.hpp"
#include "layouts/hilbert.hpp"
#include "layouts/tiled.hpp"
#include "kernels/stencil.hpp"
#include "tlb.hpp"
#include "trace.hpp"

// 3 TLBs at the same time: L1 and L2 with 4 KB pages, and one with 2 MB pages
struct Tlbs {
    Tlb l1{64, 4096};
    Tlb l2{1536, 4096};
    Tlb huge{32, 2 * 1024 * 1024};
    std::size_t accesses = 0;

    void access(std::uint64_t addr) {
        l1.access(addr);
        l2.access(addr);
        huge.access(addr);
        accesses++;
    }
};

template <std::size_t K, typename Layout>
void run(const char* name, const Layout& layout, std::uint32_t n,
         const std::vector<std::array<std::uint32_t, K>>& centers) {
    std::vector<double> in(layout.size()), out(layout.size());
    const char* traversals[] = {"coords", "random", "layout"};

    for (int t = 0; t < 3; t++) {
        Tlbs tlbs;
        Recorder<K, Layout, Tlbs> rec(layout, tlbs);
        if (t == 0)
            sweep_coords<K>(rec, in.data(), out.data(), n);
        else if (t == 1)
            sweep_random<K>(rec, in.data(), out.data(), centers);
        else
            sweep_layout<K>(rec, in.data(), out.data(), n);

        std::cout << name << "," << traversals[t] << "," << K << "," << n << ","
                  << tlbs.accesses << "," << tlbs.l1.get_misses() << ","
                  << tlbs.l2.get_misses() << "," << tlbs.huge.get_misses() << "\n";
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
    run<K>("hilbert", Hilbert<K>(n), n, centers);
}

int main(int argc, char** argv) {
    int k = argc > 1 ? std::atoi(argv[1]) : 3;
    int default_n = k == 2 ? 1024 : (k == 3 ? 128 : 32);
    std::uint32_t n = argc > 2 ? std::atoi(argv[2]) : default_n;

    std::cout << "layout,traversal,k,n,accesses,tlb64_4k,tlb1536_4k,tlb32_2m\n";
    if (k == 2)
        run_all<2>(n);
    else if (k == 3)
        run_all<3>(n);
    else if (k == 4)
        run_all<4>(n);
    else
        std::cerr << "K has to be 2, 3 or 4\n";
}
