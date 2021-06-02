
#pragma once

#include <absl/status/status.h>
#include <absl/status/statusor.h>

#include <sstream>

// Compute `value`. If the status is ok, assign to variable `var`. Otherwise,
// return the status.
#define ASSIGN_OR_RETURN(var, value) \
  auto __STATUS_NAME = (value);      \
  if (!__STATUS_NAME.ok()) {         \
    return __STATUS_NAME.status();   \
  }                                  \
  var = std::move(*__STATUS_NAME)

#define RETURN_IF_ERROR(value)  \
  auto __STATUS_NAME = (value); \
  if (!__STATUS_NAME.ok()) {    \
    return __STATUS_NAME;       \
  }

// Creates string from `message` which is streamed to a stringstream.
#define STATUS_MESSAGE(message) (std::stringstream() << message).str()

// ===== Macro Evilness ===== //

#define __STATUS_CONCAT_INDIRECT(x, y) x##y
#define __STATUS_CONCAT(x, y) __STATUS_CONCAT_INDIRECT(x, y)

#define __STATUS_NAME __STATUS_CONCAT(status_, __LINE__)
