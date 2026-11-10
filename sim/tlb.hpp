#pragma once

#include <cstddef>
#include <cstdint>
#include <list>
#include <unordered_map>

// simple TLB: fully associative, LRU
class Tlb {
public:
    Tlb(std::size_t entries, std::uint64_t page_size)
        : entries(entries), page_size(page_size), hits(0), misses(0) {}

    void access(std::uint64_t addr) {
        std::uint64_t page = addr / page_size;
        auto found = where.find(page);
        if (found != where.end()) {
            hits++;
            // move the page to the front (most recently used)
            order.splice(order.begin(), order, found->second);
            return;
        }

        misses++;
        if (order.size() == entries) {
            where.erase(order.back());
            order.pop_back();
        }
        order.push_front(page);
        where[page] = order.begin();
    }

    std::size_t get_hits() const { return hits; }
    std::size_t get_misses() const { return misses; }

private:
    std::size_t entries;
    std::uint64_t page_size;
    std::size_t hits;
    std::size_t misses;
    std::list<std::uint64_t> order;  // front = most recent page
    std::unordered_map<std::uint64_t, std::list<std::uint64_t>::iterator> where;
};
