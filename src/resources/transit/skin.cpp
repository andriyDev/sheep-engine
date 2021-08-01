
#pragma once

#include "resources/transit/skin.h"

#include <fstream>

#include "utility/hton_extra.h"
#include "utility/status.h"

namespace transit {

absl::StatusOr<std::shared_ptr<Skin>> LoadSkin(
    const TransitSkinDetails& details) {
  std::shared_ptr<Skin> skin(new Skin());
  ASSIGN_OR_RETURN((skin->skeleton), details.skeleton.Get());

  std::ifstream file(details.file, std::ios_base::binary | std::ios_base::in);
  if (!file.is_open()) {
    return absl::FailedPreconditionError(
        STATUS_MESSAGE("Failed to open file \"" << details.file << "\""));
  }

  ASSIGN_OR_RETURN((const TransitHeader& header), ReadHeader(file));
  RETURN_IF_ERROR((VerifyHeader(header, "SKIN", 1, 0)));
  ASSIGN_OR_RETURN((const nlohmann::json& json_data),
                   ReadJson(file, header.json_length));
  ASSIGN_OR_RETURN((const std::vector<unsigned char>& data),
                   ReadData(file, header.data_length));
  ASSIGN_OR_RETURN((const unsigned int vertices_count),
                   (json::GetRequiredUint(json_data, "vertices")));

  if (header.data_length != vertices_count * sizeof(Skin::Vertex)) {
    return absl::FailedPreconditionError(
        STATUS_MESSAGE("Skin data is wrong size. Expected "
                       << vertices_count * sizeof(Skin::Vertex)
                       << " bytes, but got " << header.data_length));
  }

  skin->vertices.resize(vertices_count);
  file.read((char*)skin->vertices.data(),
            vertices_count * sizeof(Skin::Vertex));
  for (Skin::Vertex& vertex : skin->vertices) {
    vertex.weights = btoh(vertex.weights);
    vertex.bone_indices.x = btoh(vertex.bone_indices.x);
    vertex.bone_indices.y = btoh(vertex.bone_indices.y);
    vertex.bone_indices.z = btoh(vertex.bone_indices.z);
    vertex.bone_indices.w = btoh(vertex.bone_indices.w);
  }

  return skin;
}

}  // namespace transit
