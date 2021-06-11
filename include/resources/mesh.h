
#pragma once

#include <GL/glew.h>
#include <absl/status/statusor.h>

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

#include "utility/resource_handle.h"

class Mesh {
 public:
  struct Vertex {
    glm::vec3 position;
    glm::vec2 texCoord;
    glm::vec3 colour;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;
  };

  struct Triangle {
    unsigned int points[3];
  };
  struct SmallTriangle {
    unsigned short points[3];
  };

  std::vector<Vertex> vertices;
  std::vector<Triangle> triangles;
  std::vector<SmallTriangle> small_triangles;
};

class RenderableMesh {
 public:
  struct Details {
    ResourceHandle<Mesh> mesh;
  };
  using detail_type = Details;

  static absl::StatusOr<std::shared_ptr<RenderableMesh>> Load(
      const Details& details);

  virtual ~RenderableMesh();

  void Draw();

 private:
  std::vector<GLuint> buffers;
  GLuint vao = 0;
  unsigned int elements;
  unsigned int vertex_attribute_count;
  bool use_small_indexing = false;
};
