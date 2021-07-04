
#pragma once

#include <functional>

// Stores a value of type `ContainedType` that must be kept up to date. If
// `clear_on_invalidate` is true, the value will be set to the default
// constructed value of `ContainedType`. Caching occurs silently.
template <typename ContainedType, bool clear_on_invalidate = false>
struct Cached {
 public:
  Cached(std::function<ContainedType()> compute_function_)
      : compute_function(compute_function_) {}

  // Invalidates the value contained within.
  void Invalidate();

  // Returns the contained value. If the value is currently invalid, computes
  // the value.
  const ContainedType& Get() const;

  // Sets the value of the cache to `value`.
  void Set(const ContainedType& value);

  const ContainedType& operator*() const { return Get(); }
  const ContainedType* operator->() const { return &Get(); };

 private:
  mutable ContainedType data;
  mutable bool valid = false;

  std::function<ContainedType()> compute_function;
};

// ===== Template Implementation ===== //

template <typename ContainedType, bool clear_on_invalidate>
void Cached<ContainedType, clear_on_invalidate>::Invalidate() {
  valid = false;
  if (clear_on_invalidate) {
    data = ContainedType();
  }
}

template <typename ContainedType, bool clear_on_invalidate>
const ContainedType& Cached<ContainedType, clear_on_invalidate>::Get() const {
  if (valid) {
    return data;
  }
  data = compute_function();
  valid = true;
  return data;
}

template <typename ContainedType, bool clear_on_invalidate>
void Cached<ContainedType, clear_on_invalidate>::Set(
    const ContainedType& value) {
  data = value;
  valid = true;
}
