
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

  // Performs an update every frame. Occurs at the start of a frame.
  // `delta_seconds` is the amount of time passed for this frame.
  virtual void Update(float delta_seconds) {}
  // Performs an update at a fixed rate. Can occur multiple times in one frame
  // to "catch up". Occurs in the middle of a frame. `delta_seconds` is the
  // amount of time each fixed frame takes.
  virtual void FixedUpdate(float delta_seconds) {}
  // Performs an update every frame. Occurs at the end of a frame.
  // `delta_seconds` is the amount of time passed for this frame.
  virtual void LateUpdate(float delta_seconds) {}

 private:
  std::weak_ptr<World> world;

  friend class World;
};
