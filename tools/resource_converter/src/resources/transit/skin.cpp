
#include "resources/skin.h"

#include "resources/transit/transit_write.h"
#include "utility/hton_extra.h"

namespace transit {

template <>
absl::Status Save(std::ostream& stream, const std::shared_ptr<Skin>& skin) {
  json::json json_data;
  json_data["vertices"] = skin->vertices.size();
  std::stringstream json_ss;
  json_ss << json_data;
  const std::string& json_string = json_ss.str();

  TransitHeader header = CreateHeader("MESH");
  header.json_length = json_string.length();
  header.data_length = sizeof(Skin::Vertex) * skin->vertices.size();
  RETURN_IF_ERROR(WriteHeader(stream, header));
  stream.write(json_string.c_str(), json_string.length());

  std::vector<Skin::Vertex> skin_data(skin->vertices);
  for (Skin::Vertex& vertex : skin_data) {
    vertex.weights = htob(vertex.weights);
    vertex.bone_indices.x = htob(vertex.bone_indices.x);
    vertex.bone_indices.y = htob(vertex.bone_indices.y);
    vertex.bone_indices.z = htob(vertex.bone_indices.z);
    vertex.bone_indices.w = htob(vertex.bone_indices.w);
  }
  stream.write((char*)skin_data.data(),
               sizeof(Skin::Vertex) * skin_data.size());

  if (stream.bad()) {
    return absl::FailedPreconditionError("Failed to write mesh to stream");
  }
  return absl::OkStatus();
}

}  // namespace transit
