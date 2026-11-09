#undef NDEBUG
#include <cassert>
#include <iostream>

#include "layouts/row_major.hpp"

template <std::size_t K>
void test_row_major(unsigned n) {
    RowMajor<K> layout(n);
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
        test_row_major<1>(n);
        test_row_major<2>(n);
        test_row_major<3>(n);
        test_row_major<4>(n);
    }

    RowMajor<3> r(4);
    assert(r.encode({0, 0, 1}) == 1);
    assert(r.encode({0, 1, 0}) == 4);
    assert(r.encode({1, 0, 0}) == 16);

    std::cout << "ok\n";
}
