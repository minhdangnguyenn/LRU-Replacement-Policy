#include "../include/fifo-replacer.h"

FIFOReplacer::FIFOReplacer() {
    uhead = new Page();
    utail = new Page();
    uhead->unext = utail;
    utail->uprev = uhead;
}

// Remove from eviction candidates (page is pinned again)
void FIFOReplacer::pin(Page *page) {
    if (unpinned_map.find(page->getKey()) == unpinned_map.end())
        return;

    remove_unpin(page);
    unpinned_map.erase(page->getKey());
}

// Add to eviction candidates — FIFO: insert at tail (oldest stays at tail)
void FIFOReplacer::unpin(Page *page) {
    if (unpinned_map.count(page->getKey()))
        return; // already queued

    // insert at head side → tail side is oldest (FIFO order)
    page->unext = uhead->unext;
    page->uprev = uhead;
    uhead->unext->uprev = page;
    uhead->unext = page;

    unpinned_map[page->getKey()] = page;
}

// O(1) — evict the oldest unpinned page (utail->uprev)
Page *FIFOReplacer::evict() {
    Page *victim = utail->uprev;
    if (victim == uhead)
        return nullptr; // all pinned

    remove_unpin(victim);
    unpinned_map.erase(victim->getKey());
    return victim;
}

void FIFOReplacer::remove_unpin(Page *p) {
    p->uprev->unext = p->unext;
    p->unext->uprev = p->uprev;
}

FIFOReplacer::~FIFOReplacer() {
    delete uhead;
    delete utail;
}
