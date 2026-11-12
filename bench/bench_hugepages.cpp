// stencil with normal 4KB pages vs 2MB huge pages
// usage: bench_hugepages [K] [n]
#include <sys/mman.h>

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "layouts/row_major.hpp"
#include "layouts/morton_fast.hpp"
#include "kernels/stencil.hpp"
#include "pages.hpp"
#include "timing.hpp"

const int ROUNDS = 7;
const std::size_t HUGE_PAGE = 2 * 1024 * 1024;

// with THP on "always" the 4KB array could get huge pages too, so we ask for both
double* alloc(std::size_t count, bool huge) {
    std::size_t bytes = (count * sizeof(double) + HUGE_PAGE - 1) / HUGE_PAGE * HUGE_PAGE;
    void* p = std::aligned_alloc(HUGE_PAGE, bytes);
    madvise(p, bytes, huge ? MADV_HUGEPAGE : MADV_NOHUGEPAGE);

    double* a = static_cast<double*>(p);
    for (std::size_t i = 0; i < count; i++)
        a[i] = 1.0;
    return a;
}

template <std::size_t K, typename Layout>
void add_cases(std::vector<Case>& cases, const std::string& name, const Layout& layout,
               std::uint32_t n, double* in, double* out,
               const std::vector<std::array<std::uint32_t, K>>& centers) {
    cases.push_back({name + ",random", [&layout, in, out, &centers] {
        sweep_random<K>(layout, in, out, centers);
    }, {}});
    cases.push_back({name + ",coords", [&layout, in, out, n] {
        sweep_coords<K>(layout, in, out, n);
    }, {}});
}

template <std::size_t K>
void run_all(std::uint32_t n) {
    RowMajor<K> row_major(n);
    MortonFast<K> morton(n);
    std::size_t size = row_major.size();

    double* in_4k = alloc(size, false);
    double* out_4k = alloc(size, false);
    double* in_2m = alloc(size, true);
    double* out_2m = alloc(size, true);
    std::cerr << "MB per array: " << size * sizeof(double) / (1024 * 1024) << ", "
              << huge_pages_in_use() << "\n";

    // as many random points as inner points
    std::size_t count = 1;
    for (std::size_t i = 0; i < K; i++)
        count *= n - 2;
    auto centers = random_centers<K>(n, count, 1);

    std::vector<Case> cases;
    add_cases<K>(cases, "row_major,4k", row_major, n, in_4k, out_4k, centers);
    add_cases<K>(cases, "row_major,2m", row_major, n, in_2m, out_2m, centers);
    add_cases<K>(cases, "morton_fast,4k", morton, n, in_4k, out_4k, centers);
    add_cases<K>(cases, "morton_fast,2m", morton, n, in_2m, out_2m, centers);
    run_in_turns(cases, ROUNDS);

    for (const auto& c : cases)
        std::cout << c.name << "," << K << "," << n << "," << c.times[0] << ","
                  << c.times[ROUNDS / 2] << "\n";
    std::cerr << "after: " << huge_pages_in_use() << "\n";

    std::free(in_4k);
    std::free(out_4k);
    std::free(in_2m);
    std::free(out_2m);
}

int main(int argc, char** argv) {
    int k = argc > 1 ? std::atoi(argv[1]) : 3;
    int default_n = k == 2 ? 4096 : k == 3 ? 256 : k == 4 ? 64 : k == 5 ? 32 : 16;
    std::uint32_t n = argc > 2 ? std::atoi(argv[2]) : default_n;

    std::cout << "layout,pages,traversal,k,n,min_ms,median_ms\n";
    if (k == 2)
        run_all<2>(n);
    else if (k == 3)
        run_all<3>(n);
    else if (k == 4)
        run_all<4>(n);
    else if (k == 5)
        run_all<5>(n);
    else if (k == 6)
        run_all<6>(n);
    else
        std::cerr << "K has to be between 2 and 6\n";
}
