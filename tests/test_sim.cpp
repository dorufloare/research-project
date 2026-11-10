#undef NDEBUG
#include <cassert>
#include <iostream>
#include <vector>

#include "layouts/row_major.hpp"
#include "kernels/stencil.hpp"
#include "tlb.hpp"
#include "trace.hpp"

struct Counter {
    std::size_t count = 0;
    void access(std::uint64_t) { count++; }
};

void test_tlb() {
    Tlb tlb(2, 4096);
    tlb.access(0);     // miss
    tlb.access(100);   // same page, hit
    tlb.access(4096);  // miss
    tlb.access(8192);  // miss, page 0 is thrown out
    tlb.access(0);     // miss, page 1 is thrown out
    tlb.access(8192);  // hit
    assert(tlb.get_hits() == 2);
    assert(tlb.get_misses() == 4);
}

// the stencil reads 2K + 1 cells for every inner point
void test_recorder() {
    const std::uint32_t n = 6;
    RowMajor<3> layout(n);
    Counter counter;
    Recorder<3, RowMajor<3>, Counter> rec(layout, counter);

    std::vector<double> in(layout.size()), out(layout.size());
    sweep_coords<3>(rec, in.data(), out.data(), n);
    assert(counter.count == 4 * 4 * 4 * 7);
}

int main() {
    test_tlb();
    test_recorder();
    std::cout << "ok\n";
}
