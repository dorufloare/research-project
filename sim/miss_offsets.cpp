// runs the light stencil walk through a simulated 24MB LRU cache (64B lines)
// and for every miss looks how far ahead of the walk it is (offset = miss - current position).
// prints which share of the misses a prefetch dist cells ahead on the curve could cover
#include <array>
#include <cstdint>
#include <iostream>

#include "kernels/stencil_light.hpp"
#include "layouts/morton_fast.hpp"
#include "tlb.hpp"

const long long DISTS[] = {64, 512, 4096, 32768, 262144};

template <std::size_t K>
void analyse(const char* name, const std::array<std::uint64_t, K>& m, std::uint32_t n,
             std::size_t size) {
    Tlb cache(24 * 1024 * 1024 / 64, 64);  // a tlb with "pages" of 64 bytes is a cache
    long long covered[5] = {0, 0, 0, 0, 0};
    long long misses = 0;

    auto access = [&](std::size_t idx, std::size_t walk) {
        std::size_t before = cache.get_misses();
        cache.access(idx * sizeof(double));
        if (cache.get_misses() == before)
            return;
        misses++;
        long long offset = (long long)idx - (long long)walk;
        for (int i = 0; i < 5; i++)
            if (offset >= 0 && offset < DISTS[i])
                covered[i]++;
    };

    // same order of reads as sweep_light
    for (std::size_t idx = 0; idx < size; idx++) {
        bool inside = true;
        for (std::size_t d = 0; d < K; d++) {
            std::uint64_t x = idx & m[d];
            if (x == 0 || x == m[d])
                inside = false;
        }
        if (!inside)
            continue;

        access(idx, idx);
        for (std::size_t d = 0; d < K; d++) {
            std::uint64_t x = idx & m[d];
            std::uint64_t rest = idx & ~m[d];
            access(((x - 1) & m[d]) | rest, idx);
            access((((x | ~m[d]) + 1) & m[d]) | rest, idx);
        }
    }

    std::cout << name << "," << K << "," << n << "," << misses;
    for (int i = 0; i < 5; i++)
        std::cout << "," << (double)covered[i] / misses;
    std::cout << "\n";
}

template <std::size_t K>
void run_all(std::uint32_t n) {
    MortonFast<K> morton(n);
    analyse<K>("row_major", row_major_masks<K>(n), n, morton.size());
    analyse<K>("morton", morton.masks(), n, morton.size());
}

int main() {
    std::cout << "layout,k,n,misses,cov_64,cov_512,cov_4096,cov_32768,cov_262144\n";
    // same sizes as the benchmarks
    run_all<2>(4096);
    run_all<3>(256);
    run_all<4>(64);
    run_all<5>(32);
    run_all<6>(16);
}
