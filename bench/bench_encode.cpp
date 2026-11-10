// times only encode, no memory accesses to the array
#include <array>
#include <iostream>
#include <vector>

#include "layouts/row_major.hpp"
#include "layouts/morton.hpp"
#include "layouts/morton_fast.hpp"
#include "layouts/hilbert.hpp"
#include "layouts/tiled.hpp"
#include "kernels/stencil.hpp"
#include "timing.hpp"

std::size_t check = 0; 

template <std::size_t K, typename Layout>
void run(const char* name, const Layout& layout, std::uint32_t n,
         const std::vector<std::array<std::uint32_t, K>>& points) {
    double ms = time_it([&] {
        std::size_t sum = 0;
        for (const auto& c : points)
            sum += layout.encode(c);
        check += sum;
    });
    double ns_per_encode = ms * 1e6 / points.size();
    std::cout << name << "," << K << "," << n << "," << ns_per_encode << "\n";
}

template <std::size_t K>
void run_all(std::uint32_t n) {
    auto points = random_centers<K>(n, 1 << 20, 1);

    run<K>("row_major", RowMajor<K>(n), n, points);
    run<K>("tiled", Tiled<K>(n), n, points);
    run<K>("morton", Morton<K>(n), n, points);
    run<K>("morton_fast", MortonFast<K>(n), n, points);
    run<K>("hilbert", Hilbert<K>(n), n, points);
}

int main() {
    std::uint32_t n = 256;
    std::cout << "layout,k,n,ns_per_encode\n";
    run_all<2>(n);
    run_all<3>(n);
    run_all<4>(n);
    run_all<5>(n);
    std::cerr << "check: " << check << "\n";
}
