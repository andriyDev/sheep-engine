
#pragma once

#include <memory>
#include <string>
#include <vector>

class World;

class Node : public std::enable_shared_from_this<Node> {
 public:
  std::string name;

  // Attaches this node to `parent` at the specified `index` safely. If this
  // node has another parent, `child` will be detached from them before
  // attaching to `parent`. If `parent` is null, this node is just detached from
  // its current parent and `index` is ignored. Negative `index`s index from the
  // end.
  void AttachTo(const std::shared_ptr<Node>& parent, int index = -1);

  // Returns every node (excluding this) from this to the root.
  std::vector<std::shared_ptr<Node>> GetAncestry() const;

  std::shared_ptr<World> GetWorld() const;
  std::shared_ptr<Node> GetParent() const;
  const std::vector<std::shared_ptr<Node>>& GetChildren() const;

 protected:
  // Attaches `child` to this node at the specified `index`. Negative `index`s
  // index from the end. Crashes if `child` has a parent.
  virtual void AttachNode(const std::shared_ptr<Node>& child, int index = -1);
  // Detaches `child` from this node. Crashes if `child` is not a child of this
  // node.
  virtual void DetachNode(const std::shared_ptr<Node>& child);

  // Notifies this node that its `root_ancestor` has attached to a `new_parent`.
  // This is called after attachment has occurred, and after its ancestors have
  // been notified. `root_ancestor` will also have this called on it.
  virtual void NotifyOfAncestorAttachment(
      const std::shared_ptr<Node>& new_parent,
      const std::shared_ptr<Node>& root_ancestor) {}
  // Notifies this node that its `root_ancestor` has detached from its `parent`.
  // This is called before detachment occurs, and after its ancestors have
  // been notified. `root_ancestor` will also have this called on it.
  virtual void NotifyOfAncestorDetachment(
      const std::shared_ptr<Node>& parent,
      const std::shared_ptr<Node>& root_ancestor) {}

 private:
  std::weak_ptr<World> world;

  std::weak_ptr<Node> parent;
  std::vector<std::shared_ptr<Node>> children;

  friend class World;
};
