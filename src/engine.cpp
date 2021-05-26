
#include "engine.h"

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

const std::vector<std::shared_ptr<World>>& Engine::GetWorlds() const {
  return worlds;
}
