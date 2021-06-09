
#pragma once

#include <functional>

struct ScopeCleanup {
 public:
  ScopeCleanup() {}
  ScopeCleanup(const std::function<void()>& routine_);
  // Performs cleanup routine.
  ~ScopeCleanup();

  // Executes the cleanup routine. On destruction, will not perform cleanup.
  void execute();

  // Clears the cleanup routine. On destruction, will not perform cleanup.
  void release();

 private:
  std::function<void()> routine;
};
