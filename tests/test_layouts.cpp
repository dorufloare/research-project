#undef NDEBUG
#include <cassert>
#include <iostream>

#include "layouts/row_major.hpp"
#include "layouts/morton.hpp"
#include "layouts/hilbert.hpp"
#include "layouts/tiled.hpp"

// decode every index, encode it back, we should get the same index
template <std::size_t K, typename Layout>
void test_layout(const Layout& layout, unsigned n) {
    for (std::size_t idx = 0; idx < layout.size(); idx++) {
        auto c = layout.decode(idx);
        for (std::size_t i = 0; i < K; i++)
            assert(c[i] < n);
        assert(layout.encode(c) == idx);
    }
}

// for hilbert: 2 consecutive indexes must be neighbours in the grid
template <std::size_t K, typename Layout>
void test_neighbours(const Layout& layout) {
    for (std::size_t idx = 1; idx < layout.size(); idx++) {
        auto a = layout.decode(idx - 1);
        auto b = layout.decode(idx);
        unsigned dist = 0;
        for (std::size_t i = 0; i < K; i++)
            dist += a[i] > b[i] ? a[i] - b[i] : b[i] - a[i];
        assert(dist == 1);
    }
}

void test_row_major() {
    // n doesn't have to be a power of 2
    for (unsigned n : {1, 2, 3, 5, 8}) {
        test_layout<1>(RowMajor<1>(n), n);
        test_layout<2>(RowMajor<2>(n), n);
        test_layout<3>(RowMajor<3>(n), n);
        test_layout<4>(RowMajor<4>(n), n);
    }

    RowMajor<3> r(4);
    assert(r.encode({0, 0, 1}) == 1);
    assert(r.encode({0, 1, 0}) == 4);
    assert(r.encode({1, 0, 0}) == 16);
}

void test_morton() {
    for (unsigned n : {1, 2, 4, 8}) {
        test_layout<1>(Morton<1>(n), n);
        test_layout<2>(Morton<2>(n), n);
        test_layout<3>(Morton<3>(n), n);
        test_layout<4>(Morton<4>(n), n);
    }

    // values checked by hand on the 4x4 grid
    Morton<2> m(4);
    assert(m.encode({0, 1}) == 1);
    assert(m.encode({1, 0}) == 2);
    assert(m.encode({0, 2}) == 4);
    assert(m.encode({2, 0}) == 8);
    assert(m.encode({3, 3}) == 15);
}

void test_hilbert() {
    for (unsigned n : {1, 2, 4, 8}) {
        test_layout<1>(Hilbert<1>(n), n);
        test_layout<2>(Hilbert<2>(n), n);
        test_layout<3>(Hilbert<3>(n), n);
        test_layout<4>(Hilbert<4>(n), n);

        test_neighbours<1>(Hilbert<1>(n));
        test_neighbours<2>(Hilbert<2>(n));
        test_neighbours<3>(Hilbert<3>(n));
        test_neighbours<4>(Hilbert<4>(n));
    }

    // starts in the corner
    Hilbert<3> h(8);
    assert(h.decode(0) == (std::array<std::uint32_t, 3>{0, 0, 0}));
}

void test_tiled() {
    for (unsigned n : {1, 2, 4, 8, 12}) {
        for (unsigned t : {1, 2, 3, 4, 8}) {
            if (n % t != 0)  // t must divide n
                continue;
            test_layout<1>(Tiled<1>(n, t), n);
            test_layout<2>(Tiled<2>(n, t), n);
            test_layout<3>(Tiled<3>(n, t), n);
            test_layout<4>(Tiled<4>(n, t), n);
        }
    }

    // 4x4 grid, 2x2 tiles
    Tiled<2> tl(4, 2);
    assert(tl.encode({0, 1}) == 1);
    assert(tl.encode({1, 0}) == 2);
    assert(tl.encode({1, 1}) == 3);
    assert(tl.encode({0, 2}) == 4);
    assert(tl.encode({2, 0}) == 8);

    // t = n is the same as row major
    Tiled<3> one_tile(4, 4);
    RowMajor<3> r(4);
    assert(one_tile.encode({1, 2, 3}) == r.encode({1, 2, 3}));

    // default t
    test_layout<3>(Tiled<3>(8), 8);
}

int main() {
    test_row_major();
    test_morton();
    test_hilbert();
    test_tiled();
    std::cout << "ok\n";
}
