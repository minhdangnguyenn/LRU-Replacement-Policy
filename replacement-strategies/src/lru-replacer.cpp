#include "../include/lru-replacer.h"

LRUReplacer::LRUReplacer() {
    uhead = new Page();
    utail = new Page();
    uhead->unext = utail;
    utail->uprev = uhead;
}

void LRUReplacer::unpin(Page *p) {
    if (unpinnedMap.count(p->getKey()))
        return;

    unpinnedMap[p->getKey()] = p;

    p->unext = uhead->unext;
    p->uprev = uhead;
    uhead->unext->uprev = p;
    uhead->unext = p;
}

void LRUReplacer::pin(Page *p) {
    if (unpinnedMap.find(p->getKey()) == unpinnedMap.end())
        return;

    removeUnpin(p);
    unpinnedMap.erase(p->getKey());
}

Page *LRUReplacer::evict() {
    Page *victim = utail->uprev;
    if (victim == uhead)
        return nullptr; // all pinned

    removeUnpin(victim);
    unpinnedMap.erase(victim->getKey());
    return victim;
}

void LRUReplacer::removeUnpin(Page *p) {
    p->uprev->unext = p->unext;
    p->unext->uprev = p->uprev;
}

LRUReplacer::~LRUReplacer() {
    delete uhead;
    delete utail;
}
