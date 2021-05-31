
#pragma once

#include <memory>

#include "systems/system.h"

class Engine;
class World;

class SuperSystem : public std::enable_shared_from_this<SuperSystem> {
 public:
  std::shared_ptr<Engine> GetEngine() const;

 protected:
  // Performs initialization of the super system. Occurs when Engine::Init has
  // been called, or when added to the engine after Engine::Init. Occurs before
  // any Systems have been initialized.
  virtual void Init() {}

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

  // Notifies this super system that `world` has been created. Called after
  // world has been created.
  virtual void NotifyOfWorldCreation(const std::shared_ptr<World>& world) {}
  // Notifies this super system that `world` has been created. Called before
  // world has been removed.
  virtual void NotifyOfWorldDeletion(const std::shared_ptr<World>& world) {}
  // Notifies this super system that `system` has been added to `world`. Called
  // after system has been added.
  virtual void NotifyOfSystemAddition(const std::shared_ptr<World>& world,
                                      const std::shared_ptr<System>& system) {}
  // Notifies this super system that `system` has been removed from `world`.
  // Called before system has been removed.
  virtual void NotifyOfSystemRemoval(const std::shared_ptr<World>& world,
                                     const std::shared_ptr<System>& system) {}

 private:
  std::weak_ptr<Engine> engine;

  friend class Engine;
};
