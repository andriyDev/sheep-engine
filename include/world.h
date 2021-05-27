
#pragma once

#include <memory>
#include <vector>

#include "nodes/node.h"
#include "system.h"

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
  World();

  std::shared_ptr<Node> root;
  std::vector<std::shared_ptr<System>> systems;

  std::weak_ptr<Engine> engine;

  friend class Engine;
};
