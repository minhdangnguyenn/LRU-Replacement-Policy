#include "../include/b-plus-tree.h"
#include <iostream>

BPlusTree::BPlusTree(BufferPool *bp, int inner_cap)
    : buffer_pool(bp), inner_cap(inner_cap), root_page_id(-1) {
  root_page_id = buffer_pool->create_new_page();
}

int BPlusTree::lookup(int key) {
  std::cout << "NOT IMPLEMENTED YET" << std::endl;
  return 0;
}

void BPlusTree::insert(int key, int page_id) {
  // std::cout << "NOT IMPLEMENTED YET" << std::endl;
  char *data = this->buffer_pool->fetch_page(page_id);
}

void BPlusTree::remove(int key) {
  std::cout << "NOT IMPLEMENTED YET" << std::endl;
}

void BPlusTree::range_scan(int low, int high, std::vector<int> &results) {
  std::cout << "NOT IMPLEMENTED YET" << std::endl;
}
