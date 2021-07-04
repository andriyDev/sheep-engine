
#pragma once

#include <glm/glm.hpp>
#include <vector>

class Skeleton {
 public:
  std::vector<glm::mat4> inverse_bind_matrices;
};
