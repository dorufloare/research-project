#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

// row major, like a normal C array a[x0][x1]...
// last coordinate moves fastest in memory
template <std::size_t K>
class RowMajor {
public:
    RowMajor(std::uint32_t n) : n(n) {}

    std::size_t size() const {
        std::size_t s = 1;
        for (std::size_t i = 0; i < K; i++)
            s *= n;
        return s;
    }

    std::size_t encode(const std::array<std::uint32_t, K>& c) const {
        std::size_t idx = 0;
        for (std::size_t i = 0; i < K; i++)
            idx = idx * n + c[i];
        return idx;
    }

    std::array<std::uint32_t, K> decode(std::size_t idx) const {
        std::array<std::uint32_t, K> c{};
        for (std::size_t i = K; i > 0; i--) {
            c[i - 1] = idx % n;
            idx /= n;
        }
        return c;
    }

private:
    std::uint32_t n;
};
