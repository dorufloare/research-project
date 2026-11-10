#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

// grid cut in tiles of side t, row major between tiles and inside each tile
// t must divide n
template <std::size_t K>
class Tiled {
public:
    Tiled(std::uint32_t n, std::uint32_t t = 4) : n(n), t(t) {
        tiles = n / t;  // tiles per side
        tile_size = 1;  // t^K
        for (std::size_t i = 0; i < K; i++)
            tile_size *= t;
    }

    std::size_t size() const {
        std::size_t s = 1;
        for (std::size_t i = 0; i < K; i++)
            s *= n;
        return s;
    }

    std::size_t encode(const std::array<std::uint32_t, K>& c) const {
        std::size_t tile = 0;
        std::size_t inside = 0;
        for (std::size_t i = 0; i < K; i++) {
            tile = tile * tiles + c[i] / t;
            inside = inside * t + c[i] % t;
        }
        return tile * tile_size + inside;
    }

    std::array<std::uint32_t, K> decode(std::size_t idx) const {
        std::size_t tile = idx / tile_size;
        std::size_t inside = idx % tile_size;
        std::array<std::uint32_t, K> c{};
        for (std::size_t i = K; i > 0; i--) {
            c[i - 1] = (tile % tiles) * t + inside % t;
            tile /= tiles;
            inside /= t;
        }
        return c;
    }

private:
    std::uint32_t n;
    std::uint32_t t;
    std::size_t tiles;
    std::size_t tile_size;
};
