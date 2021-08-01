
#include "resources/transit/transit_write.h"

#include <glog/logging.h>

#include "utility/hton.h"

namespace transit {

TransitHeader CreateHeader(const char* type) {
  return {{'T', 'R', 'S', 'T'}, {type[0], type[1], type[2], type[3]}, {1, 0}};
}

absl::Status WriteHeader(std::ostream& stream, const TransitHeader& header) {
  TransitHeader mut_header(header);
  mut_header.data_length = htob(mut_header.data_length);
  mut_header.json_length = htob(mut_header.json_length);
  stream.write((char*)&mut_header, sizeof(TransitHeader));
  if (stream.bad()) {
    return absl::UnknownError("Failed to write header to stream.");
  }
  return absl::OkStatus();
}

}  // namespace transit
