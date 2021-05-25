
#include "nodes/node.h"

#include <assert.h>
#include <stdio.h>

void Node::AttachNode(const std::shared_ptr<Node>& child, int index) {
  assert(child.get());
  assert(!child->GetParent().get());
  child->parent = this->shared_from_this();
  if (index < 0) {
    index += children.size() + 1;
    assert(index >= 0);
  } else {
    assert(index < children.size());
  }
  children.insert(children.begin() + index, child);
}

void Node::DetachNode(const std::shared_ptr<Node>& child) {
  assert(child.get());
  assert(child->GetParent().get() == this);
  child->parent = std::shared_ptr<Node>();
  const auto node_it = std::find(children.begin(), children.end(), child);
  children.erase(node_it);
}

void Node::AdoptNode(const std::shared_ptr<Node>& child, int index) {
  const std::shared_ptr<Node> parent = child->GetParent();
  if (parent) {
    parent->DetachNode(child);
  }
  AttachNode(child, index);
}

std::vector<std::shared_ptr<Node>> Node::GetAncestry() const {
  std::vector<std::shared_ptr<Node>> ancestors;
  std::shared_ptr<Node> current_node = this->GetParent();
  while (current_node) {
    ancestors.push_back(current_node);
    current_node = current_node->GetParent();
  }
  return ancestors;
}

std::shared_ptr<Node> Node::GetParent() const { return parent.lock(); }

const std::vector<std::shared_ptr<Node>>& Node::GetChildren() const {
  return children;
}
