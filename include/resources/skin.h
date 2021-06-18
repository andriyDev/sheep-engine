
#pragma once

#include <GL/glew.h>
#include <absl/status/statusor.h>

#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "utility/resource_handle.h"

class Skin {
 public:
  struct Vertex {
    float weights[4];
    uint32_t bone_indices[4];
  };

  std::vector<Vertex> vertices;
};
