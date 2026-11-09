#undef NDEBUG
#include <cassert>
#include <iostream>

#include "layouts/row_major.hpp"
#include "layouts/morton.hpp"
#include "layouts/hilbert.hpp"

template <std::size_t K, typename Layout>
void test_layout(const Layout& layout, unsigned n) {
    for (std::size_t idx = 0; idx < layout.size(); idx++) {
        auto c = layout.decode(idx);
        for (std::size_t i = 0; i < K; i++)
            assert(c[i] < n);
        assert(layout.encode(c) == idx);
    }
}

// hilbert only: the points at idx - 1 and idx must be neighbours in the grid
// (they differ by 1 in exactly one coordinate)
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

int main() {
    // not only powers of 2
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

    // morton needs powers of 2
    for (unsigned n : {1, 2, 4, 8}) {
        test_layout<1>(Morton<1>(n), n);
        test_layout<2>(Morton<2>(n), n);
        test_layout<3>(Morton<3>(n), n);
        test_layout<4>(Morton<4>(n), n);
    }

    Morton<2> m(4);
    assert(m.encode({0, 1}) == 1);
    assert(m.encode({1, 0}) == 2);
    assert(m.encode({0, 2}) == 4);
    assert(m.encode({2, 0}) == 8);
    assert(m.encode({3, 3}) == 15);

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

    // curve starts in the corner
    Hilbert<3> h(8);
    assert(h.decode(0) == (std::array<std::uint32_t, 3>{0, 0, 0}));

    std::cout << "ok\n";
}
