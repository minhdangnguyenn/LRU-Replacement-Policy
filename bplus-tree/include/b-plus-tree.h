#ifndef B_TREE
#define B_TREE

#include "./buffer-pool.h"
#include "./index-strategy.h"
#include <vector>
class BPlusTree : public IndexStrategy {
public:
  BPlusTree(BufferPool *bp, int node_capacity);
  int lookup(int key) override;
  void insert(int key, int page_id) override;
  void remove(int key) override;
  void range_scan(int low, int high, std::vector<int> &results) override;

  enum class NodeType { LEAF, INTERNAL };

  struct Node {
    NodeType type;
    std::vector<int> keys;
    std::vector<Node> children;
    Node *next_node;
    Node(NodeType type = NodeType::LEAF) : type(type) {}
  };

private:
  BufferPool *buffer_pool;
  int root_page_id;
  int inner_cap;
};

#endif // !B_TREE
