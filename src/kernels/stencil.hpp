#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

// true if the point is not on the border of the grid
template <std::size_t K>
bool is_inside(const std::array<std::uint32_t, K>& c, std::uint32_t n) {
    for (std::size_t i = 0; i < K; i++)
        if (c[i] == 0 || c[i] == n - 1)
            return false;
    return true;
}

// sum of the cell and its 2K neighbours, c must not be on the border
// always_inline: gcc inlined this for some layouts and K and not for others,
// and a call per cell made the coords sweep up to 3x slower
template <std::size_t K, typename Layout>
[[gnu::always_inline]] inline void apply_stencil(const Layout& layout, const double* in, double* out,
                   std::array<std::uint32_t, K> c) {
    std::size_t center = layout.encode(c);
    double sum = in[center];
    for (std::size_t d = 0; d < K; d++) {
        c[d]--;
        sum += in[layout.encode(c)];
        c[d] += 2;
        sum += in[layout.encode(c)];
        c[d]--;
    }
    out[center] = sum;
}

// moves c to the next inner point, last coordinate fastest
// returns false when there are no more points
template <std::size_t K>
bool next_point(std::array<std::uint32_t, K>& c, std::uint32_t n) {
    for (std::size_t d = K; d > 0; d--) {
        c[d - 1]++;
        if (c[d - 1] < n - 1)
            return true;
        c[d - 1] = 1;
    }
    return false;
}

// go through the points in coordinate order
template <std::size_t K, typename Layout>
void sweep_coords(const Layout& layout, const double* in, double* out, std::uint32_t n) {
    std::array<std::uint32_t, K> c;
    c.fill(1);
    do {
        apply_stencil<K>(layout, in, out, c);
    } while (next_point<K>(c, n));
}

// random points, made before the timing starts
template <std::size_t K>
std::vector<std::array<std::uint32_t, K>> random_centers(std::uint32_t n, std::size_t count,
                                                         unsigned seed) {
    std::mt19937 gen(seed);
    std::uniform_int_distribution<std::uint32_t> dist(1, n - 2);
    std::vector<std::array<std::uint32_t, K>> centers(count);
    for (auto& c : centers)
        for (std::size_t i = 0; i < K; i++)
            c[i] = dist(gen);
    return centers;
}

template <std::size_t K, typename Layout>
void sweep_random(const Layout& layout, const double* in, double* out,
                  const std::vector<std::array<std::uint32_t, K>>& centers) {
    for (const auto& c : centers)
        apply_stencil<K>(layout, in, out, c);
}

// go through the points in memory order (index 0, 1, 2...)
template <std::size_t K, typename Layout>
void sweep_layout(const Layout& layout, const double* in, double* out, std::uint32_t n) {
    for (std::size_t idx = 0; idx < layout.size(); idx++) {
        auto c = layout.decode(idx);
        if (is_inside<K>(c, n))
            apply_stencil<K>(layout, in, out, c);
    }
}
