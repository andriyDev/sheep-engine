
#include "system.h"

std::shared_ptr<World> System::GetWorld() const { return world.lock(); }
