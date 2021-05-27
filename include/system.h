
#pragma once

#include <memory>

#include "nodes/node.h"

class World;

class System : public std::enable_shared_from_this<System> {
 public:
  std::shared_ptr<World> GetWorld() const;

 protected:
  // Handles `new_node` being attached to the node tree. This is called after
  // the node is attached. Systems are not notified of child nodes of
  // `new_node`.
  virtual void NotifyOfNodeAttachment(const std::shared_ptr<Node>& new_node) {}
  // Handles `new_node` being detached from the node tree. This is called before
  // the node is detached. Systems are not notified of child nodes of
  // `new_node`.
  virtual void NotifyOfNodeDetachment(const std::shared_ptr<Node>& new_node) {}

 private:
  std::weak_ptr<World> world;

  friend class World;
};
