#pragma once

#include <cstddef>

// arrays we prefetch in get this many extra elements at the end,
// so we never need an if before the prefetch
const std::size_t PREFETCH_PAD = 32768;

// memory is in curve order, so dist elements ahead = dist cells ahead on the curve
// 2 = load into L2, the L1 is too small for lines this far ahead
inline void prefetch_along_curve(const double* in, std::size_t idx, std::size_t dist) {
    __builtin_prefetch(in + idx + dist, 0, 2);
}
