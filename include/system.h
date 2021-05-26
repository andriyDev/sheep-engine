
#pragma once

#include <memory>

class World;

class System : public std::enable_shared_from_this<System> {
 public:
  std::shared_ptr<World> GetWorld() const;

 private:
  std::weak_ptr<World> world;

  friend class World;
};
