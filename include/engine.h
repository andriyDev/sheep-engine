
#pragma once

#include <memory>
#include <vector>

#include "world.h"

class Engine : public std::enable_shared_from_this<Engine> {
 public:
  std::shared_ptr<World> CreateWorld();
  void RemoveWorld(const std::shared_ptr<World>& world);

  const std::vector<std::shared_ptr<World>>& GetWorlds() const;

 private:
  std::vector<std::shared_ptr<World>> worlds;
};
