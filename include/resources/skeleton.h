
#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>

#include "utility/cached.h"
#include "utility/status.h"

class Skeleton {
 public:
  struct Bone {
    std::string name;
    struct Pose {
      glm::vec3 position;
      glm::quat rotation;
      glm::vec3 scale;
    } bind_pose;
    std::vector<unsigned int> children;
  };

  std::vector<Bone> bones;
  Cached<std::vector<glm::mat4>> inverse_bind_matrices;

  Skeleton();

  std::vector<Bone::Pose> GetBindPose() const;

  absl::StatusOr<std::vector<glm::mat4>> ComputePoseMatrices(
      const std::vector<Bone::Pose>& poses) const;

  absl::StatusOr<std::vector<glm::mat4>> ComputeRelativePoseMatrices(
      const std::vector<Bone::Pose>& poses) const;

 private:
  std::vector<glm::mat4> ComputeInverseBindMatrices() const;
};
