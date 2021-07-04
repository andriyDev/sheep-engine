
#pragma once

#include <GL/glew.h>
#include <absl/status/statusor.h>

#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "resources/skeleton.h"
#include "utility/resource_handle.h"

class Skin {
 public:
  struct Vertex {
    glm::vec4 weights;
    glm::vec<4, unsigned short> bone_indices;
  };

  std::vector<Vertex> vertices;
  std::shared_ptr<Skeleton> skeleton;
};
