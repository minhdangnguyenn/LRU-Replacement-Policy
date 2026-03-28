#include "page.h"
#ifndef LRU_H
#define LRU_H
#include "replacer.h"
#include <unordered_map>
#include <list>

class LRU : public Replacer {
public:
    LRU(size_t num_pages);
    ~LRU() override = default;
    bool victim(int& frame_id) override;
    void pin(int, int);
    void unpin(int);
    size_t Size();
private:
    std::list<int> orders;
    std::unordered_map<int, std::list<int>::iterator> candidates;
};

#endif //LRU_H