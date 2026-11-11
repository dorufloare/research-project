#undef NDEBUG
#include <cassert>
#include <iostream>
#include <vector>

#include "layouts/row_major.hpp"
#include "layouts/morton.hpp"
#include "layouts/morton_fast.hpp"
#include "layouts/hilbert.hpp"
#include "layouts/tiled.hpp"
#include "kernels/stencil.hpp"
#include "kernels/stencil_light.hpp"

// value of a point, doesn't depend on the layout
template <std::size_t K>
double value(const std::array<std::uint32_t, K>& c) {
    unsigned v = 0;
    for (std::size_t i = 0; i < K; i++)
        v = v * 13 + c[i];
    return v % 101;
}

// what the stencil should give for a point
template <std::size_t K>
double expected(std::array<std::uint32_t, K> c) {
    double sum = value<K>(c);
    for (std::size_t d = 0; d < K; d++) {
        c[d]--;
        sum += value<K>(c);
        c[d] += 2;
        sum += value<K>(c);
        c[d]--;
    }
    return sum;
}

// borders stay -1, everything inside has to match expected()
template <std::size_t K, typename Layout>
void check_sweep(const Layout& layout, const std::vector<double>& out, std::uint32_t n) {
    for (std::size_t idx = 0; idx < layout.size(); idx++) {
        auto c = layout.decode(idx);
        if (is_inside<K>(c, n))
            assert(out[idx] == expected<K>(c));
        else
            assert(out[idx] == -1);
    }
}

template <std::size_t K, typename Layout>
void test_stencil(const Layout& layout, std::uint32_t n) {
    std::size_t total = layout.size();
    std::vector<double> in(total);
    for (std::size_t idx = 0; idx < total; idx++)
        in[idx] = value<K>(layout.decode(idx));

    std::vector<double> out(total, -1);
    sweep_coords<K>(layout, in.data(), out.data(), n);
    check_sweep<K>(layout, out, n);

    std::fill(out.begin(), out.end(), -1);
    sweep_layout<K>(layout, in.data(), out.data(), n);
    check_sweep<K>(layout, out, n);

    std::fill(out.begin(), out.end(), -1);
    auto centers = random_centers<K>(n, 50, 1);
    sweep_random<K>(layout, in.data(), out.data(), centers);
    for (const auto& c : centers)
        assert(out[layout.encode(c)] == expected<K>(c));
}

// the light stencil must give the same result as the normal one, with and without prefetch
template <std::size_t K, typename Layout>
void test_light(const Layout& layout, const std::array<std::uint64_t, K>& m, std::uint32_t n) {
    for (std::size_t dist : {0, 64, 32768}) {
        std::vector<double> in(layout.size() + PREFETCH_PAD), out(layout.size(), -1);
        for (std::size_t idx = 0; idx < layout.size(); idx++)
            in[idx] = value<K>(layout.decode(idx));
        sweep_light<K>(m, layout.size(), in.data(), out.data(), dist);
        check_sweep<K>(layout, out, n);
    }
}

int main() {
    test_stencil<2>(RowMajor<2>(5), 5);
    test_stencil<3>(RowMajor<3>(5), 5);

    test_stencil<2>(Morton<2>(8), 8);
    test_stencil<3>(Morton<3>(8), 8);
    test_stencil<2>(Hilbert<2>(8), 8);
    test_stencil<3>(Hilbert<3>(8), 8);
    test_stencil<2>(Tiled<2>(8, 4), 8);
    test_stencil<3>(Tiled<3>(8, 4), 8);

    test_light<2>(RowMajor<2>(8), row_major_masks<2>(8), 8);
    test_light<3>(RowMajor<3>(8), row_major_masks<3>(8), 8);
    test_light<4>(RowMajor<4>(4), row_major_masks<4>(4), 4);
    MortonFast<2> m2(8);
    MortonFast<3> m3(8);
    MortonFast<4> m4(4);
    test_light<2>(m2, m2.masks(), 8);
    test_light<3>(m3, m3.masks(), 8);
    test_light<4>(m4, m4.masks(), 4);

    std::cout << "ok\n";
}
