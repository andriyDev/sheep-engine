
#include "nodes/utility.h"

#include <functional>

void TraversePreOrder(const std::shared_ptr<Node>& node,
                      std::vector<std::shared_ptr<Node>>& result) {
  result.push_back(node);
  for (const std::shared_ptr<Node>& child : node->GetChildren()) {
    TraversePreOrder(child, result);
  }
}

std::vector<std::shared_ptr<Node>> CollectPreOrderNodes(
    const std::shared_ptr<Node>& root) {
  std::vector<std::shared_ptr<Node>> nodes;
  TraversePreOrder(root, nodes);
  return nodes;
}

void TraversePostOrder(const std::shared_ptr<Node>& node,
                       std::vector<std::shared_ptr<Node>>& result) {
  for (const std::shared_ptr<Node>& child : node->GetChildren()) {
    TraversePostOrder(child, result);
  }
  result.push_back(node);
}

std::vector<std::shared_ptr<Node>> CollectPostOrderNodes(
    const std::shared_ptr<Node>& root) {
  std::vector<std::shared_ptr<Node>> nodes;
  TraversePostOrder(root, nodes);
  return nodes;
}
