// average number of cache lines and pages one stencil touches
#include <algorithm>
#include <array>
#include <iostream>

#include "layouts/row_major.hpp"
#include "layouts/morton.hpp"
#include "layouts/morton_fast.hpp"
#include "layouts/hilbert.hpp"
#include "layouts/tiled.hpp"
#include "kernels/stencil.hpp"

// 64 byte line, 4 KB page, 8 bytes per double
const std::size_t DOUBLES_PER_LINE = 8;
const std::size_t DOUBLES_PER_PAGE = 512;

// how many different blocks the indexes fall in
template <std::size_t N>
std::size_t count_blocks(std::array<std::size_t, N> idx, std::size_t block) {
    for (auto& i : idx)
        i /= block;
    std::sort(idx.begin(), idx.end());
    return std::unique(idx.begin(), idx.end()) - idx.begin();
}

template <std::size_t K, typename Layout>
void footprint(const char* name, const Layout& layout, std::uint32_t n) {
    double lines = 0, pages = 0;
    std::size_t cells = 0;

    for (std::size_t i = 0; i < layout.size(); i++) {
        auto c = layout.decode(i);
        if (!is_inside<K>(c, n))
            continue;

        std::array<std::size_t, 2 * K + 1> idx;
        idx[0] = layout.encode(c);
        for (std::size_t d = 0; d < K; d++) {
            c[d]--;
            idx[2 * d + 1] = layout.encode(c);
            c[d] += 2;
            idx[2 * d + 2] = layout.encode(c);
            c[d]--;
        }

        lines += count_blocks(idx, DOUBLES_PER_LINE);
        pages += count_blocks(idx, DOUBLES_PER_PAGE);
        cells++;
    }

    std::cout << name << "," << K << "," << n << "," << lines / cells << ","
              << pages / cells << "\n";
}

template <std::size_t K>
void run_all(std::uint32_t n) {
    footprint<K>("row_major", RowMajor<K>(n), n);
    footprint<K>("tiled", Tiled<K>(n), n);
    footprint<K>("morton", Morton<K>(n), n);
    footprint<K>("morton_fast", MortonFast<K>(n), n);
    footprint<K>("hilbert", Hilbert<K>(n), n);
}

int main() {
    std::cout << "layout,k,n,lines,pages\n";
    // same sizes as the default ones in bench_stencil
    run_all<2>(4096);
    run_all<3>(256);
    run_all<4>(64);
    run_all<5>(32);
    run_all<6>(16);
}
