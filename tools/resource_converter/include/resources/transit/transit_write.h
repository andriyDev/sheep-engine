
#pragma once

#include <ostream>

#include "resources/transit/transit.h"
#include "utility/status.h"

namespace transit {

// Saves `resource` to `stream` in the transit format.
template <typename ResourceType>
absl::Status Save(std::ostream& stream,
                  const std::shared_ptr<ResourceType>& resource);

// Creates a header for the specified type with the version and transit id
// filled in.
TransitHeader CreateHeader(const char* type);

// Writes `header` to `stream`, ensuring byte order.
absl::Status WriteHeader(std::ostream& stream, const TransitHeader& header);

}  // namespace transit
