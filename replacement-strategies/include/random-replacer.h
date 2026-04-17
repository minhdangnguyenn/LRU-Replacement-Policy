#pragma once
#include "replacer.h"
#include <unordered_map>
#include <vector>

class RandomReplacer : public Replacer {
  public:
    RandomReplacer();
    void pin(Page *page) override;
    void unpin(Page *page) override;
    Page *evict() override;

  private:
    // store all unpinned pages as a flat list
    std::vector<Page *> unpinned_list;
    std::unordered_map<int, int> index_map; // key -> index in unpinned_list
};
