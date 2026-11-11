#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "prefetch/curve_prefetch.hpp"

// with n a power of 2, every coordinate is a group of bits inside the index:
// row major = the bits of a coordinate are next to each other, morton = mixed.
// m[d] marks the bits of coordinate d, so one kernel works for both layouts.

template <std::size_t K>
std::array<std::uint64_t, K> row_major_masks(std::uint32_t n) {
    std::uint64_t bits = 0;
    while ((1u << bits) < n)
        bits++;
    std::array<std::uint64_t, K> m;
    for (std::size_t d = 0; d < K; d++)
        m[d] = std::uint64_t(n - 1) << (bits * (K - 1 - d));
    return m;
}

// stencil with very little work per cell, walking the memory in order
// dist = how far ahead to prefetch (0 = none), one prefetch per 8 cells (a cache line)
// size has to be a multiple of 8
template <std::size_t K>
void sweep_light(const std::array<std::uint64_t, K>& m, std::size_t size,
                 const double* in, double* out, std::size_t dist) {
    for (std::size_t block = 0; block < size; block += 8) {
        if (dist > 0)
            prefetch_along_curve(in, block, dist);

        for (std::size_t idx = block; idx < block + 8; idx++) {
            // border = coordinate is 0 (no bits set) or n-1 (all bits set)
            bool inside = true;
            for (std::size_t d = 0; d < K; d++) {
                std::uint64_t x = idx & m[d];
                if (x == 0 || x == m[d])
                    inside = false;
            }
            if (!inside)
                continue;

            double sum = in[idx];
            for (std::size_t d = 0; d < K; d++) {
                std::uint64_t x = idx & m[d];
                std::uint64_t rest = idx & ~m[d];
                // x | ~m fills the gaps with 1s, so the +1 carry jumps over them
                std::uint64_t plus = (((x | ~m[d]) + 1) & m[d]) | rest;
                std::uint64_t minus = ((x - 1) & m[d]) | rest;
                sum += in[minus] + in[plus];
            }
            out[idx] = sum;
        }
    }
}
