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

const int ROUNDS = 15;
std::size_t check = 0;

template <std::size_t K, typename Layout>
void add_case(std::vector<Case>& cases, const std::string& name, const Layout& layout,
              const std::vector<std::array<std::uint32_t, K>>& points) {
    cases.push_back({name, [&layout, &points] {
        std::size_t sum = 0;
        for (const auto& c : points)
            sum += layout.encode(c);
        check += sum;
    }, {}});
}

template <std::size_t K>
void run_all(std::uint32_t n) {
    RowMajor<K> row_major(n);
    Tiled<K> tiled(n);
    Morton<K> morton(n);
    MortonFast<K> morton_fast(n);
    Hilbert<K> hilbert(n);
    auto points = random_centers<K>(n, 1 << 20, 1);

    std::vector<Case> cases;
    add_case<K>(cases, "row_major", row_major, points);
    add_case<K>(cases, "tiled", tiled, points);
    add_case<K>(cases, "morton", morton, points);
    add_case<K>(cases, "morton_fast", morton_fast, points);
    add_case<K>(cases, "hilbert", hilbert, points);
    run_in_turns(cases, ROUNDS);

    for (const auto& c : cases) {
        double min_ns = c.times[0] * 1e6 / points.size();
        double median_ns = c.times[ROUNDS / 2] * 1e6 / points.size();
        std::cout << c.name << "," << K << "," << n << "," << min_ns << "," << median_ns << "\n";
    }
}

int main() {
    std::uint32_t n = 256;
    std::cout << "layout,k,n,min_ns,median_ns\n";
    run_all<2>(n);
    run_all<3>(n);
    run_all<4>(n);
    run_all<5>(n);
    run_all<6>(n);
    std::cerr << "check: " << check << "\n";
}
