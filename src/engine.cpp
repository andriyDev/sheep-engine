
#include "engine.h"

#include <assert.h>

std::shared_ptr<World> Engine::CreateWorld() {
  std::shared_ptr<World> world(new World());
  world->engine = this->shared_from_this();
  worlds.push_back(world);
  return world;
}

void Engine::RemoveWorld(const std::shared_ptr<World>& world) {
  const auto it = std::find(worlds.begin(), worlds.end(), world);
  if (it != worlds.end()) {
    worlds.erase(it);
  }
}

void Engine::AddSuperSystem(const std::shared_ptr<SuperSystem>& super_system,
                            int index) {
  // Only allow adding `super_system` if it is not a part of some engine.
  assert(!super_system->GetEngine().get());
  super_system->engine = this->shared_from_this();
  if (index < 0) {
    index += super_systems.size() + 1;
    assert(index >= 0);
  } else {
    assert(index < super_systems.size());
  }
  super_systems.insert(super_systems.begin() + index, super_system);
}

void Engine::RemoveSuperSystem(
    const std::shared_ptr<SuperSystem>& super_system) {
  const auto it =
      std::find(super_systems.begin(), super_systems.end(), super_system);
  if (it != super_systems.end()) {
    super_systems.erase(it);
  }
}

const std::vector<std::shared_ptr<World>>& Engine::GetWorlds() const {
  return worlds;
}
const std::vector<std::shared_ptr<SuperSystem>>& Engine::GetSuperSystems()
    const {
  return super_systems;
}

void Engine::Update(float delta_seconds) {
  for (const std::shared_ptr<World>& world : worlds) {
    world->Update(delta_seconds);
  }
  for (const std::shared_ptr<SuperSystem>& super_system : super_systems) {
    super_system->Update(delta_seconds);
  }
}

void Engine::FixedUpdate(float delta_seconds) {
  for (const std::shared_ptr<World>& world : worlds) {
    world->FixedUpdate(delta_seconds);
  }
  for (const std::shared_ptr<SuperSystem>& super_system : super_systems) {
    super_system->FixedUpdate(delta_seconds);
  }
}

void Engine::LateUpdate(float delta_seconds) {
  for (const std::shared_ptr<World>& world : worlds) {
    world->LateUpdate(delta_seconds);
  }
  for (const std::shared_ptr<SuperSystem>& super_system : super_systems) {
    super_system->LateUpdate(delta_seconds);
  }
}
