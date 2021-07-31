
#pragma once

#include "resources/mesh.h"

#include <fstream>

#include "resources/transit/transit.h"
#include "utility/hton_extra.h"

namespace transit {

template <>
absl::StatusOr<std::shared_ptr<Mesh>> Load(const TransitDetails& details) {
  std::ifstream file(details.file);
  if (!file.is_open()) {
    return absl::FailedPreconditionError(
        STATUS_MESSAGE("Failed to open file \"" << details.file << "\""));
  }

  ASSIGN_OR_RETURN((const TransitHeader& header), ReadHeader(file));
  RETURN_IF_ERROR((VerifyHeader(header, "MESH", 1, 0)));
  ASSIGN_OR_RETURN((const nlohmann::json& json_data),
                   ReadJson(file, header.json_length));
  ASSIGN_OR_RETURN((const std::vector<unsigned char>& data),
                   ReadData(file, header.data_length));
  ASSIGN_OR_RETURN((const unsigned int vertices_count),
                   (json::GetRequiredUint(json_data, "vertices")));
  ASSIGN_OR_RETURN((const std::string& indexing_mode_str),
                   json::GetRequiredString(json_data, "indexingMode"));
  if (indexing_mode_str != "small" && indexing_mode_str != "big") {
    return absl::FailedPreconditionError(STATUS_MESSAGE(
        "Invalid indexing mode. Expected: one of small, big. Actual: "
        << indexing_mode_str));
  }
  bool indexing_mode_is_big = indexing_mode_str == "big";
  ASSIGN_OR_RETURN((const unsigned int triangles),
                   json::GetRequiredUint(json_data, "triangles"));

  unsigned int expected_data_size =
      sizeof(Mesh::Vertex) * vertices_count +
      (indexing_mode_is_big ? sizeof(Mesh::Triangle)
                            : sizeof(Mesh::SmallTriangle)) *
          triangles;
  if (header.data_length != expected_data_size) {
    return absl::FailedPreconditionError(STATUS_MESSAGE(
        "Recieved data size does not match expected data size. Expected: "
        << expected_data_size << ", Actual: " << header.data_length));
  }

  std::shared_ptr<Mesh> mesh(new Mesh());
  mesh->vertices.reserve(vertices_count);
  Mesh::Vertex* data_vertex = (Mesh::Vertex*)data.data();
  for (int i = 0; i < vertices_count; i++, data_vertex++) {
    Mesh::Vertex& new_vertex = mesh->vertices.emplace_back();
    memcpy(&new_vertex, data_vertex, sizeof(Mesh::Vertex));
    new_vertex.position = btoh(new_vertex.position);
    new_vertex.normal = btoh(new_vertex.normal);
    new_vertex.colour = btoh(new_vertex.colour);
    new_vertex.texCoord = btoh(new_vertex.texCoord);
    new_vertex.tangent = btoh(new_vertex.tangent);
    new_vertex.bitangent = btoh(new_vertex.bitangent);
  }
  if (indexing_mode_is_big) {
    mesh->triangles.reserve(triangles);
    Mesh::Triangle* data_triangle =
        (Mesh::Triangle*)(data.data() + vertices_count * sizeof(Mesh::Vertex));
    for (int i = 0; i < triangles; i++, data_triangle++) {
      Mesh::Triangle& new_triangle = mesh->triangles.emplace_back();
      memcpy(&new_triangle, data_triangle, sizeof(Mesh::Triangle));
      new_triangle.points[0] = btoh(new_triangle.points[0]);
      new_triangle.points[1] = btoh(new_triangle.points[1]);
      new_triangle.points[2] = btoh(new_triangle.points[2]);
    }
  } else {
    mesh->small_triangles.reserve(triangles);
    Mesh::SmallTriangle* data_triangle =
        (Mesh::SmallTriangle*)(data.data() +
                               vertices_count * sizeof(Mesh::Vertex));
    for (int i = 0; i < triangles; i++, data_triangle++) {
      Mesh::SmallTriangle& new_triangle = mesh->small_triangles.emplace_back();
      memcpy(&new_triangle, data_triangle, sizeof(Mesh::SmallTriangle));
      new_triangle.points[0] = btoh(new_triangle.points[0]);
      new_triangle.points[1] = btoh(new_triangle.points[1]);
      new_triangle.points[2] = btoh(new_triangle.points[2]);
    }
  }
  return mesh;
}

}  // namespace transit
