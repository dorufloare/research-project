#pragma once

#include <immintrin.h>

#include <array>
#include <cstddef>
#include <cstdint>

// same layout as Morton, but with the pdep / pext instructions (BMI2)
// n must be a power of 2
template <std::size_t K>
class MortonFast {
public:
    MortonFast(std::uint32_t n) : n(n) {
        std::size_t bits = 0;
        while ((1u << bits) < n)
            bits++;

        // mask[d] has a 1 on the places where the bits of coordinate d go
        for (std::size_t d = 0; d < K; d++) {
            mask[d] = 0;
            for (std::size_t b = 0; b < bits; b++)
                mask[d] |= 1ull << (b * K + (K - 1 - d));
        }
    }

    std::size_t size() const {
        std::size_t s = 1;
        for (std::size_t i = 0; i < K; i++)
            s *= n;
        return s;
    }

    std::size_t encode(const std::array<std::uint32_t, K>& c) const {
        std::uint64_t idx = 0;
        for (std::size_t d = 0; d < K; d++)
            idx |= _pdep_u64(c[d], mask[d]);
        return idx;
    }

    // pext is the opposite of pdep: takes the bits under the mask and packs them
    std::array<std::uint32_t, K> decode(std::size_t idx) const {
        std::array<std::uint32_t, K> c{};
        for (std::size_t d = 0; d < K; d++)
            c[d] = _pext_u64(idx, mask[d]);
        return c;
    }

    const std::array<std::uint64_t, K>& masks() const { return mask; }

private:
    std::uint32_t n;
    std::array<std::uint64_t, K> mask;
};
