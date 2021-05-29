
#include "nodes/node.h"

#include <assert.h>
#include <stdio.h>

#include "nodes/utility.h"
#include "world.h"

void Node::AttachNode(const std::shared_ptr<Node>& child, int index) {
  assert(child.get());
  assert(!child->GetParent().get());
  child->world = GetWorld();
  child->parent = this->shared_from_this();
  if (index < 0) {
    index += children.size() + 1;
    assert(index >= 0);
  } else {
    assert(index < children.size());
  }
  children.insert(children.begin() + index, child);

  for (const std::shared_ptr<Node>& node : CollectPreOrderNodes(child)) {
    node->NotifyOfAncestorAttachment(this->shared_from_this(), child);
  }

  const std::shared_ptr<World> world = GetWorld();
  if (world) {
    world->PropagateNodeAttachment(child);
  }
}

void Node::DetachNode(const std::shared_ptr<Node>& child) {
  assert(child.get());
  assert(child->GetParent().get() == this);

  for (const std::shared_ptr<Node>& node : CollectPreOrderNodes(child)) {
    node->NotifyOfAncestorDetachment(this->shared_from_this(), child);
  }

  const std::shared_ptr<World> world = GetWorld();
  if (world) {
    world->PropagateNodeAttachment(child);
  }

  child->world = std::shared_ptr<World>();
  child->parent = std::shared_ptr<Node>();
  const auto node_it = std::find(children.begin(), children.end(), child);
  children.erase(node_it);
}

void Node::AttachTo(const std::shared_ptr<Node>& parent, int index) {
  const std::shared_ptr<Node> old_parent = GetParent();
  if (old_parent) {
    old_parent->DetachNode(this->shared_from_this());
  }
  if (parent) {
    parent->AttachNode(this->shared_from_this(), index);
  }
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

std::shared_ptr<World> Node::GetWorld() const { return world.lock(); }

std::shared_ptr<Node> Node::GetParent() const { return parent.lock(); }

const std::vector<std::shared_ptr<Node>>& Node::GetChildren() const {
  return children;
}
