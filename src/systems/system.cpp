
#include "systems/system.h"

std::shared_ptr<World> System::GetWorld() const { return world.lock(); }

std::shared_ptr<Engine> System::GetEngine() const { return engine.lock(); }
