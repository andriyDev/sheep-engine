
#pragma once

#include <memory>
#include <string>
#include <vector>

class World;

class Node : public std::enable_shared_from_this<Node> {
 public:
  std::string name;

  // Attaches `child` to this node at the specified `index`. Negative `index`s
  // index from the end. Crashes if `child` has a parent.
  virtual void AttachNode(const std::shared_ptr<Node>& child, int index = -1);
  // Detaches `child` from this node. Crashes if `child` is not a child of this
  // node.
  virtual void DetachNode(const std::shared_ptr<Node>& child);

  // Attaches `child` to this node at the specified `index` safely. If `child`
  // has another parent, `child` will be detached from them before attaching to
  // this node. Negative `index`s index from the end.
  void AdoptNode(const std::shared_ptr<Node>& child, int index = -1);

  // Returns every node (excluding this) from this to the root.
  std::vector<std::shared_ptr<Node>> GetAncestry() const;

  std::shared_ptr<World> GetWorld() const;
  std::shared_ptr<Node> GetParent() const;
  const std::vector<std::shared_ptr<Node>>& GetChildren() const;

 private:
  std::weak_ptr<World> world;

  std::weak_ptr<Node> parent;
  std::vector<std::shared_ptr<Node>> children;

  friend class World;
};
