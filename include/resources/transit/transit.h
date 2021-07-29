
#pragma once

#include <memory>
#include <nlohmann/json.hpp>
#include <string>

#include "utility/status.h"

namespace transit {

// Defines the details for all transit files.
struct TransitDetails {
  std::string file;
};

// Loads a transit resource provided its details.
template <typename ResourceType>
absl::StatusOr<std::shared_ptr<ResourceType>> Load(
    const TransitDetails& details);

// Defines the header of transit files.
struct TransitHeader {
  // Bytes to identify whether or not this is a transit file.
  char transit_id[4];
  // Bytes to identify the type of the contents of the file.
  char type_id[4];
  // Version code of transit.
  unsigned char version[2];
  // Length of the JSON data.
  unsigned int json_length;
  // Length of the binary data.
  unsigned int data_length;
};

// Reads the header of a transit file from a stream. If header is badly
// formatted, still consumes header size bytes.
absl::StatusOr<TransitHeader> ReadHeader(std::istream& stream);

// Verifies whether the header has expected values.
absl::Status VerifyHeader(const TransitHeader& header,
                          const char* expected_type,
                          unsigned char version_major,
                          unsigned char version_minor);

absl::StatusOr<nlohmann::json> ReadJson(std::istream& stream,
                                        unsigned int length);
absl::StatusOr<std::vector<unsigned char>> ReadData(std::istream& stream,
                                                    unsigned int length);

}  // namespace transit
