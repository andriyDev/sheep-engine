
#pragma once

#include <memory>
#include <vector>

#include "world.h"

class Engine : public std::enable_shared_from_this<Engine> {
 public:
  std::shared_ptr<World> CreateWorld();
  void RemoveWorld(const std::shared_ptr<World>& world);

  // Performs an update on all worlds every frame. Occurs at the start of a
  // frame. `delta_seconds` is the amount of time passed for this frame.
  void Update(float delta_seconds);
  // Performs an update on all worlds at a fixed rate. Can occur multiple times
  // in one frame to "catch up". Occurs in the middle of a frame.
  // `delta_seconds` is the amount of time each fixed frame takes.
  void FixedUpdate(float delta_seconds);
  // Performs an update on all worlds every frame. Occurs at the end of a frame.
  // `delta_seconds` is the amount of time passed for this frame.
  void LateUpdate(float delta_seconds);

  const std::vector<std::shared_ptr<World>>& GetWorlds() const;

 private:
  std::vector<std::shared_ptr<World>> worlds;
};
