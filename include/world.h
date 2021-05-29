
#pragma once

#include <memory>
#include <vector>

#include "nodes/node.h"
#include "systems/system.h"

class Engine;

class World : public std::enable_shared_from_this<World> {
 public:
  // Sets the root to `new_root`.
  void SetRoot(const std::shared_ptr<Node>& new_root);
  // Adds `new_system` to this world at `index`. Negative values of `index`
  // index from the end.
  void AddSystem(const std::shared_ptr<System>& new_system, int index = -1);
  // Removes `system` from this world. No error if `system` is not in this
  // world.
  void RemoveSystem(const std::shared_ptr<System>& system);

  std::shared_ptr<Node> GetRoot() const;
  std::shared_ptr<Engine> GetEngine() const;
  const std::vector<std::shared_ptr<System>>& GetSystems() const;

 private:
  // Performs an update on all systems every frame. Occurs at the start of a
  // frame. `delta_seconds` is the amount of time passed for this frame.
  void Update(float delta_seconds);
  // Performs an update on all systems at a fixed rate. Can occur multiple times
  // in one frame to "catch up". Occurs in the middle of a frame.
  // `delta_seconds` is the amount of time each fixed frame takes.
  void FixedUpdate(float delta_seconds);
  // Performs an update on all systems every frame. Occurs at the end of a
  // frame. `delta_seconds` is the amount of time passed for this frame.
  void LateUpdate(float delta_seconds);

  void PropagateNodeAttachment(const std::shared_ptr<Node>& node);
  void PropagateNodeDetachment(const std::shared_ptr<Node>& node);

  std::shared_ptr<Node> root;
  std::vector<std::shared_ptr<System>> systems;

  std::weak_ptr<Engine> engine;

  friend class Engine;
  friend class Node;
};
