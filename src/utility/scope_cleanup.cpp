
#include "utility/scope_cleanup.h"

ScopeCleanup::ScopeCleanup(const std::function<void()>& routine_)
    : routine(routine_) {}

ScopeCleanup::~ScopeCleanup() { execute(); }

void ScopeCleanup::execute() {
  if (routine) {
    routine();
    release();
  }
}

void ScopeCleanup::release() { routine = std::function<void()>(); }
