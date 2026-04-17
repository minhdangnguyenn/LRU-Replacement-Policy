#include "../include/random-replacer.h"
#include <cstdlib>

RandomReplacer::RandomReplacer() {
    std::srand(42); // fixed seed for reproducibility in benchmarks
}

void RandomReplacer::pin(Page *page) {
    auto it = index_map.find(page->getKey());
    if (it == index_map.end())
        return;

    // swap with last element then pop — O(1) removal
    int idx = it->second;
    Page *last = unpinned_list.back();

    unpinned_list[idx] = last;
    index_map[last->getKey()] = idx;

    unpinned_list.pop_back();
    index_map.erase(page->getKey());
}

void RandomReplacer::unpin(Page *page) {
    if (index_map.count(page->getKey()))
        return;

    index_map[page->getKey()] = unpinned_list.size();
    unpinned_list.push_back(page);
}

Page *RandomReplacer::evict() {
    if (unpinned_list.empty())
        return nullptr;

    // pick a random index
    int idx = std::rand() % unpinned_list.size();
    Page *victim = unpinned_list[idx];

    // swap with last then pop — O(1)
    Page *last = unpinned_list.back();
    unpinned_list[idx] = last;
    index_map[last->getKey()] = idx;

    unpinned_list.pop_back();
    index_map.erase(victim->getKey());

    return victim;
}
