
#include "systems/super_system.h"

std::shared_ptr<Engine> SuperSystem::GetEngine() const { return engine.lock(); }
