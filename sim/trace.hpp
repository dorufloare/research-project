#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

// wraps a layout, every time encode is called the address of that
// element (as a double) is sent to the tracker
template <std::size_t K, typename Layout, typename Tracker>
class Recorder {
public:
    Recorder(const Layout& layout, Tracker& tracker) : layout(layout), tracker(tracker) {}

    std::size_t size() const { return layout.size(); }

    std::size_t encode(const std::array<std::uint32_t, K>& c) const {
        std::size_t idx = layout.encode(c);
        tracker.access(idx * sizeof(double));
        return idx;
    }

    std::array<std::uint32_t, K> decode(std::size_t idx) const {
        return layout.decode(idx);
    }

private:
    const Layout& layout;
    Tracker& tracker;
};
