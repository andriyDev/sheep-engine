
#pragma once

#include <absl/container/flat_hash_set.h>

#include <memory>

#include "nodes/node.h"
#include "nodes/utility.h"
#include "systems/system.h"

template <typename TargetType, typename SourceType>
struct TypeGroup {
 public:
  using ContainerType =
      typename absl::flat_hash_set<std::shared_ptr<TargetType>>;

  // Adds several `elements` to this group.
  void Add(const std::vector<std::shared_ptr<SourceType>>& elements);

  // Removes several `elements` from this group.
  void Remove(const std::vector<std::shared_ptr<SourceType>>& elements);

  typename ContainerType::const_iterator begin() const {
    return cast_elements.begin();
  }
  typename ContainerType::const_iterator end() const {
    return cast_elements.end();
  }

  typename ContainerType::const_iterator cbegin() const {
    return cast_elements.cbegin();
  }
  typename ContainerType::const_iterator cend() const {
    return cast_elements.cend();
  }

 private:
  ContainerType cast_elements;
};

template <typename NodeType>
struct NodeTypeGroup : public TypeGroup<NodeType, Node> {
 public:
  void Add(const std::vector<std::shared_ptr<Node>>& nodes) {
    TypeGroup<NodeType, Node>::Add(nodes);
  }

  void Remove(const std::vector<std::shared_ptr<Node>>& nodes) {
    TypeGroup<NodeType, Node>::Remove(nodes);
  }

  // Adds a tree of nodes with `root` to this group.
  void AddTree(const std::shared_ptr<Node>& root);

  // Removes a tree of nodes with `root` from this group.
  void RemoveTree(const std::shared_ptr<Node>& root);
};

template <typename SystemType>
struct SystemTypeGroup : public TypeGroup<SystemType, System> {
 public:
  void Add(const std::vector<std::shared_ptr<System>>& systems) {
    TypeGroup<SystemType, System>::Add(systems);
  }

  void Remove(const std::vector<std::shared_ptr<System>>& systems) {
    TypeGroup<SystemType, System>::Remove(systems);
  }

  // Adds a `system` to this group.
  void AddSystem(const std::shared_ptr<System>& system);

  // Removes a `system` from this group.
  void RemoveSystem(const std::shared_ptr<System>& system);
};

// ===== Template Implementation ===== //

template <typename TargetType, typename SourceType>
void TypeGroup<TargetType, SourceType>::Add(
    const std::vector<std::shared_ptr<SourceType>>& elements) {
  for (const std::shared_ptr<SourceType>& element : elements) {
    const auto cast_element = std::dynamic_pointer_cast<TargetType>(element);
    if (cast_element) {
      cast_elements.insert(cast_element);
    }
  }
}

template <typename TargetType, typename SourceType>
void TypeGroup<TargetType, SourceType>::Remove(
    const std::vector<std::shared_ptr<SourceType>>& elements) {
  for (const std::shared_ptr<SourceType>& element : elements) {
    const auto cast_element = std::dynamic_pointer_cast<TargetType>(element);
    if (cast_element) {
      cast_elements.erase(cast_element);
    }
  }
}

template <typename NodeType>
void NodeTypeGroup<NodeType>::AddTree(const std::shared_ptr<Node>& root) {
  Add(CollectPreOrderNodes(root));
}

template <typename NodeType>
void NodeTypeGroup<NodeType>::RemoveTree(const std::shared_ptr<Node>& root) {
  Remove(CollectPreOrderNodes(root));
}

template <typename SystemType>
void SystemTypeGroup<SystemType>::AddSystem(
    const std::shared_ptr<System>& system) {
  Add({system});
}

template <typename SystemType>
void SystemTypeGroup<SystemType>::RemoveSystem(
    const std::shared_ptr<System>& system) {
  Remove({system});
}
