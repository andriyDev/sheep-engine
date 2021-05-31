
#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <memory>
#include <vector>

#include "systems/super_system.h"
#include "world.h"

class Engine : public std::enable_shared_from_this<Engine> {
 public:
  std::shared_ptr<World> CreateWorld();
  void RemoveWorld(const std::shared_ptr<World>& world);
  void InitWorld(const std::shared_ptr<World>& world);

  // Adds `super_system` to this engine at `index`. If `index` is negative,
  // indexes from the end. Returns `super_system`.
  const std::shared_ptr<SuperSystem>& AddSuperSystem(
      const std::shared_ptr<SuperSystem>& super_system, int index = -1);
  // Removes `super_system` from this engine. Crashes if `super_system` is not
  // added to this engine.
  void RemoveSuperSystem(const std::shared_ptr<SuperSystem>& super_system);

  void Run(GLFWwindow* window);

  const std::vector<std::shared_ptr<World>>& GetWorlds() const;
  const std::vector<std::shared_ptr<SuperSystem>>& GetSuperSystems() const;

  template <typename SystemType>
  std::shared_ptr<SystemType> GetSuperSystem() const;

 private:
  std::vector<std::shared_ptr<World>> worlds;
  std::vector<std::shared_ptr<SuperSystem>> super_systems;

  bool is_initialized = false;

  // Performs initialization of the engine. Occurs only once at the start of the
  // engine.
  void Init();

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

  void PropagateSystemAddition(const std::shared_ptr<World>& world,
                               const std::shared_ptr<System>& system);
  void PropagateSystemRemoval(const std::shared_ptr<World>& world,
                              const std::shared_ptr<System>& system);

  friend class World;
};

// ===== Template Implementation ===== //

template <typename SystemType>
std::shared_ptr<SystemType> Engine::GetSuperSystem() const {
  for (const std::shared_ptr<SuperSystem>& system : super_systems) {
    const auto cast_system = std::dynamic_pointer_cast<SystemType>(system);
    if (cast_system) {
      return cast_system;
    }
  }
  return nullptr;
}
