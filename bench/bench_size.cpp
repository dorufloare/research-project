// random access stencil for growing arrays, row major vs morton_fast, K=3
// prints ns per stencil, to see from which size on the curve pays off
#include <cstdlib>
#include <iostream>
#include <vector>

#include "layouts/row_major.hpp"
#include "layouts/morton_fast.hpp"
#include "kernels/stencil.hpp"
#include "pages.hpp"
#include "timing.hpp"

const int ROUNDS = 7;
const std::size_t STENCILS = 1 << 22;

void run(std::uint32_t n) {
    RowMajor<3> row_major(n);
    MortonFast<3> morton(n);
    std::vector<double> in(row_major.size(), 1.0);
    std::vector<double> out(row_major.size(), 0.0);
    auto centers = random_centers<3>(n, STENCILS, 1);

    std::vector<Case> cases;
    cases.push_back({"row_major", [&] {
        sweep_random<3>(row_major, in.data(), out.data(), centers);
    }, {}});
    cases.push_back({"morton_fast", [&] {
        sweep_random<3>(morton, in.data(), out.data(), centers);
    }, {}});
    run_in_turns(cases, ROUNDS);

    double mb = row_major.size() * sizeof(double) / (1024.0 * 1024.0);
    for (const auto& c : cases)
        std::cout << c.name << "," << n << "," << mb << "," << c.times[0] * 1e6 / STENCILS
                  << "," << c.times[ROUNDS / 2] * 1e6 / STENCILS << "\n";
}

int main() {
    small_pages_only();
    std::cout << "layout,n,mb_per_array,min_ns,median_ns\n";
    for (std::uint32_t n : {16, 32, 64, 128, 256, 512})
        run(n);
    std::cerr << huge_pages_in_use() << "\n";
}
