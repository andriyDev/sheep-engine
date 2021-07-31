
#include "resources/mesh.h"

#include <sstream>

#include "resources/transit/transit_write.h"
#include "utility/hton_extra.h"
#include "utility/json.h"

namespace transit {

template <>
absl::Status Save(std::ostream& stream, const std::shared_ptr<Mesh>& mesh) {
  json::json json_data;
  if (mesh->triangles.size() > 0 && mesh->small_triangles.size() > 0) {
    return absl::InvalidArgumentError(
        "Mesh has both big and small triangles! Only one is allowed.");
  }
  json_data["vertices"] = (unsigned int)mesh->vertices.size();
  bool indexing_mode = mesh->triangles.size() > 0;
  unsigned int triangle_count =
      indexing_mode ? mesh->triangles.size() : mesh->small_triangles.size();
  json_data["triangles"] = triangle_count;
  json_data["indexingMode"] = indexing_mode ? "big" : "small";
  std::stringstream json_ss;
  json_ss << json_data;
  const std::string& json_string = json_ss.str();

  TransitHeader header = CreateHeader("MESH");
  header.json_length = json_string.length();
  header.data_length =
      sizeof(Mesh::Vertex) * mesh->vertices.size() +
      (indexing_mode ? sizeof(Mesh::Triangle) : sizeof(Mesh::SmallTriangle)) *
          triangle_count;
  RETURN_IF_ERROR(WriteHeader(stream, header));
  stream.write(json_string.c_str(), json_string.length());
  {
    std::vector<Mesh::Vertex> vertices = mesh->vertices;
    for (Mesh::Vertex& vertex : vertices) {
      vertex.position = htob(vertex.position);
      vertex.normal = htob(vertex.normal);
      vertex.colour = htob(vertex.colour);
      vertex.texCoord = htob(vertex.texCoord);
      vertex.tangent = htob(vertex.tangent);
      vertex.bitangent = htob(vertex.bitangent);
    }
    stream.write((char*)vertices.data(),
                 sizeof(Mesh::Vertex) * vertices.size());
  }

  if (indexing_mode) {
    std::vector<Mesh::Triangle> triangles(mesh->triangles);
    for (Mesh::Triangle& triangle : triangles) {
      triangle.points[0] = htob(triangle.points[0]);
      triangle.points[1] = htob(triangle.points[1]);
      triangle.points[2] = htob(triangle.points[2]);
    }
    stream.write((char*)triangles.data(),
                 sizeof(Mesh::Triangle) * triangles.size());
  } else {
    std::vector<Mesh::SmallTriangle> triangles(mesh->small_triangles);
    for (Mesh::SmallTriangle& triangle : triangles) {
      triangle.points[0] = htob(triangle.points[0]);
      triangle.points[1] = htob(triangle.points[1]);
      triangle.points[2] = htob(triangle.points[2]);
    }
    stream.write((char*)triangles.data(),
                 sizeof(Mesh::SmallTriangle) * triangles.size());
  }
  if (stream.bad()) {
    return absl::FailedPreconditionError("Failed to write mesh to stream");
  }
  return absl::OkStatus();
}

}  // namespace transit
