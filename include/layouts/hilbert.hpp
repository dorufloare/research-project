#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "morton.hpp"

// hilbert curve, n must be a power of 2
// algorithm is from Skilling, "Programming the Hilbert curve" (2004)
// first the coordinates are changed with some xor / swap steps, after that the
// bits are interleaved like in morton, so I just call Morton for that part
template <std::size_t K>
class Hilbert {
public:
    Hilbert(std::uint32_t n) : morton(n) {
        bits = 0;
        while ((1u << bits) < n)
            bits++;
    }

    std::size_t size() const {
        return morton.size();
    }

    std::size_t encode(std::array<std::uint32_t, K> c) const {
        if (bits == 0)
            return 0;

        // from the highest bit down: if the bit is set, flip the low bits of c[0],
        // if not, swap the low bits of c[0] and c[i]
        for (std::uint32_t bit = 1u << (bits - 1); bit > 1; bit >>= 1) {
            std::uint32_t low = bit - 1;  // mask with all the bits under "bit"
            for (std::size_t i = 0; i < K; i++) {
                if (c[i] & bit) {
                    c[0] ^= low;
                } else {
                    std::uint32_t diff = (c[0] ^ c[i]) & low;
                    c[0] ^= diff;
                    c[i] ^= diff;
                }
            }
        }

        // gray code
        for (std::size_t i = 1; i < K; i++)
            c[i] ^= c[i - 1];
        std::uint32_t flip = 0;
        for (std::uint32_t bit = 1u << (bits - 1); bit > 1; bit >>= 1)
            if (c[K - 1] & bit)
                flip ^= bit - 1;
        for (std::size_t i = 0; i < K; i++)
            c[i] ^= flip;

        return morton.encode(c);
    }

    std::array<std::uint32_t, K> decode(std::size_t idx) const {
        std::array<std::uint32_t, K> c = morton.decode(idx);
        if (bits == 0)
            return c;

        // undo the gray code
        std::uint32_t last = c[K - 1] >> 1;
        for (std::size_t i = K - 1; i > 0; i--)
            c[i] ^= c[i - 1];
        c[0] ^= last;

        for (std::uint32_t bit = 2; bit < (1u << bits); bit <<= 1) {
            std::uint32_t low = bit - 1;
            for (std::size_t i = K; i > 0; i--) {
                if (c[i - 1] & bit) {
                    c[0] ^= low;
                } else {
                    std::uint32_t diff = (c[0] ^ c[i - 1]) & low;
                    c[0] ^= diff;
                    c[i - 1] ^= diff;
                }
            }
        }
        return c;
    }

private:
    Morton<K> morton;
    std::size_t bits;  // log2(n)
};
