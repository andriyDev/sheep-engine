
#pragma once

#include <memory>

class Engine;

class SuperSystem : public std::enable_shared_from_this<SuperSystem> {
 public:
  std::shared_ptr<Engine> GetEngine() const;

 protected:
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
  std::weak_ptr<Engine> engine;

  friend class Engine;
};
