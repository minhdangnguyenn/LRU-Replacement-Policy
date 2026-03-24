#ifndef LRU_CACHE_NAIVE_H
#define LRU_CACHE_NAIVE_H

#include <vector>
#include <utility>

class LRUCacheNaive {
private:
    int capacity;
    // vector of {key, value} — order = recency (front = MRU, back = LRU)
    std::vector<std::pair<int, int>> cache;

public:
    LRUCacheNaive(int capacity);
    int get(int key);
    void put(int key, int value);
};

#endif
