#undef NDEBUG
#include <cassert>
#include <iostream>

#include "layouts/row_major.hpp"
#include "layouts/morton.hpp"

template <std::size_t K, typename Layout>
void test_layout(const Layout& layout, unsigned n) {
    for (std::size_t idx = 0; idx < layout.size(); idx++) {
        auto c = layout.decode(idx);
        for (std::size_t i = 0; i < K; i++)
            assert(c[i] < n);
        assert(layout.encode(c) == idx);
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

    std::cout << "ok\n";
}
