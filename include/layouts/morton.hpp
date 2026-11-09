#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

// morton / z-order, n must be a power of 2
// idea: write each coordinate in binary and shuffle the bits together 
template <std::size_t K>
class Morton {
public:
    Morton(std::uint32_t n) : n(n) {
        bits = 0;
        while ((1u << bits) < n)
            bits++;
    }

    std::size_t size() const {
        std::size_t s = 1;
        for (std::size_t i = 0; i < K; i++)
            s *= n;
        return s;
    }

    std::size_t encode(const std::array<std::uint32_t, K>& c) const {
        std::size_t idx = 0;
        for (std::size_t b = 0; b < bits; b++)
            for (std::size_t d = 0; d < K; d++) {
                std::size_t bit = (c[d] >> b) & 1;
                idx |= bit << (b * K + (K - 1 - d));
            }
        return idx;
    }

    std::array<std::uint32_t, K> decode(std::size_t idx) const {
        std::array<std::uint32_t, K> c{};
        for (std::size_t b = 0; b < bits; b++)
            for (std::size_t d = 0; d < K; d++) {
                std::size_t bit = (idx >> (b * K + (K - 1 - d))) & 1;
                c[d] |= bit << b;
            }
        return c;
    }

private:
    std::uint32_t n;
    std::size_t bits;  // log2(n)
};
