
#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>

#include "utility/cached.h"

class Skeleton {
 public:
  struct Bone {
    std::string name;
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
    std::vector<unsigned int> children;
  };

  std::vector<Bone> bones;
  Cached<std::vector<glm::mat4>> inverse_bind_matrices;

  Skeleton();

 private:
  std::vector<glm::mat4> ComputeInverseBindMatrices() const;
};
