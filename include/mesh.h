
#pragma once

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

class Mesh {
 public:
  struct Vertex {
    glm::vec3 position;
    glm::vec2 texCoord;
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
    std::string mesh;
  };
  using detail_type = Details;

  static std::shared_ptr<RenderableMesh> Load(const Details& details);

  static std::shared_ptr<RenderableMesh> LoadFromMesh(
      const std::shared_ptr<Mesh>& source_mesh);

  virtual ~RenderableMesh();

  void Draw();

 private:
  std::vector<GLuint> buffers;
  GLuint vao = 0;
  unsigned int elements;
  unsigned int vertex_attribute_count;
  bool use_small_indexing = false;
};
