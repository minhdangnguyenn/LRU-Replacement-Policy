#include "../include/b-plus-tree.h"
#include <iostream>

BPlusTree::BPlusTree(BufferPool *bp, int inner_cap)
    : buffer_pool(bp), inner_cap(inner_cap), root_page_id(-1) {
  root_page_id = buffer_pool->create_new_page();
}

int BPlusTree::lookup(int key) {
  Node *current_node = this->nodes[this->root_page_id];
  while (current_node->type == NodeType::INNER) {
    // traverse down to find idx of child node
    int child_idx = current_node->traverse(key);
    // after traverse -> get the idx of child node (leaf node)
    current_node = this->nodes[child_idx];
  }

  // after traverse, current is a leaf node
  for (int i : current_node->keys) {
    if (current_node->keys[i] == key) {
      return current_node->value;
    }
  }

  // if not found match key -> return -1
  return -1;
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
