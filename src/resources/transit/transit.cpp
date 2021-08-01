
#include "resources/transit/transit.h"

#include "utility/hton.h"

namespace transit {

absl::StatusOr<TransitHeader> ReadHeader(std::istream& stream) {
  TransitHeader header;
  stream.read((char*)&header, sizeof(TransitHeader));
  if (stream.bad()) {
    return absl::InvalidArgumentError(
        STATUS_MESSAGE("Failed to read header bytes of transit file. eof: "
                       << stream.eof() << " fail: " << stream.fail()));
  }
  header.json_length = btoh(header.json_length);
  header.data_length = btoh(header.data_length);
  return header;
}

absl::Status VerifyHeader(const TransitHeader& header,
                          const char* expected_type,
                          unsigned char version_major,
                          unsigned char version_minor) {
  if (memcmp(header.transit_id, "TRST", 4) != 0) {
    char transit_id_str[5];
    memcpy(transit_id_str, header.transit_id, 4);
    transit_id_str[4] = 0;
    return absl::InvalidArgumentError(STATUS_MESSAGE(
        "Header has wrong magic bytes. Expected: \"TRST\", Actual: "
        << std::string(transit_id_str)));
  }
  if (memcmp(header.type_id, expected_type, 4) != 0) {
    char type_id_str[5];
    memcpy(type_id_str, expected_type, 4);
    type_id_str[4] = 0;
    return absl::InvalidArgumentError(STATUS_MESSAGE(
        "Header has different type. Expected: \""
        << expected_type << "\", Actual: \"" << type_id_str << "\""));
  }
  if (header.version[0] != version_major ||
      header.version[1] != version_minor) {
    return absl::InvalidArgumentError(STATUS_MESSAGE(
        "Header has different version. Expected: "
        << version_major << "." << version_minor
        << ", Actual: " << header.version[0] << "." << header.version[1]));
  }
  return absl::OkStatus();
}

absl::StatusOr<json::json> ReadJson(std::istream& stream, unsigned int length) {
  std::vector<uint8_t> json_data;
  json_data.resize(length);
  stream.read((char*)json_data.data(), length);
  const unsigned int read_bytes = stream.gcount();
  if (read_bytes != length) {
    return absl::InvalidArgumentError(STATUS_MESSAGE(
        "Failed to read JSON data. Expected "
        << length << " more bytes, but only got " << read_bytes));
  }
  try {
    const nlohmann::json& data = nlohmann::json::parse(json_data);
    if (!data.is_object()) {
      return absl::InvalidArgumentError("JSON data is not an object");
    }
    return data;
  } catch (int err) {
    return absl::InvalidArgumentError("Failed to parse JSON data");
  }
}

absl::StatusOr<std::vector<unsigned char>> ReadData(std::istream& stream,
                                                    unsigned int length) {
  std::vector<unsigned char> data;
  data.resize(length);
  stream.read((char*)data.data(), length);
  const unsigned int read_bytes = stream.gcount();
  if (read_bytes != length) {
    return absl::InvalidArgumentError(STATUS_MESSAGE(
        "Failed to read data. Expected "
        << length << " more bytes, but only got " << read_bytes));
  }
  return data;
}

}  // namespace transit
