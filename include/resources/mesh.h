
#pragma once

#include <glm/glm.hpp>
#include <vector>

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
