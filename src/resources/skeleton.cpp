
#include "resources/skeleton.h"

#include <absl/container/flat_hash_map.h>

#include "utility/status.h"

Skeleton::Skeleton()
    : inverse_bind_matrices(
          [this]() { return this->ComputeInverseBindMatrices(); }) {}

std::vector<Skeleton::Bone::Pose> Skeleton::GetBindPose() const {
  std::vector<Bone::Pose> pose;
  pose.reserve(bones.size());
  for (const Bone& bone : bones) {
    pose.push_back(bone.bind_pose);
  }
  return pose;
}

absl::StatusOr<std::vector<glm::mat4>> Skeleton::ComputePoseMatrices(
    const std::vector<Bone::Pose>& poses) const {
  if (poses.size() != bones.size()) {
    return absl::InvalidArgumentError(STATUS_MESSAGE(
        "Provided `poses` does not have same size as `bones`. Expected "
        << bones.size() << " poses, but got " << poses.size()));
  }
  absl::flat_hash_map<unsigned int, unsigned int> parent_map;
  for (unsigned int i = 0; i < bones.size(); i++) {
    const Bone& bone = bones[i];
    for (unsigned int child : bone.children) {
      parent_map.insert(std::make_pair(child, i));
    }
  }
  absl::flat_hash_map<unsigned int, glm::mat4> matrix_map;
  const auto compute_local_matrix = [this, &poses](unsigned int index) {
    return glm::translate(glm::identity<glm::mat4>(), poses[index].position) *
           glm::toMat4(poses[index].rotation) *
           glm::scale(glm::identity<glm::mat4>(), poses[index].scale);
  };
  const std::function<glm::mat4(unsigned int)> compute_matrix =
      [&matrix_map, &parent_map, &compute_local_matrix,
       &compute_matrix](unsigned int index) -> glm::mat4 {
    const auto matrix_it = matrix_map.find(index);
    if (matrix_it != matrix_map.end()) {
      return matrix_it->second;
    }
    glm::mat4 new_matrix = compute_local_matrix(index);
    const auto parent_it = parent_map.find(index);
    if (parent_it != parent_map.end()) {
      new_matrix = compute_matrix(parent_it->second) * new_matrix;
    }
    matrix_map.insert(std::make_pair(index, new_matrix));
    return new_matrix;
  };
  std::vector<glm::mat4> matrices;
  matrices.reserve(bones.size());
  for (unsigned int i = 0; i < bones.size(); i++) {
    matrices.push_back(compute_matrix(i));
  }
  return matrices;
}

absl::StatusOr<std::vector<glm::mat4>> Skeleton::ComputeRelativePoseMatrices(
    const std::vector<Bone::Pose>& poses) const {
  ASSIGN_OR_RETURN((std::vector<glm::mat4> matrices),
                   ComputePoseMatrices(poses));
  for (unsigned int i = 0; i < matrices.size(); i++) {
    matrices[i] = matrices[i] * (*inverse_bind_matrices)[i];
  }
  return matrices;
}

std::vector<glm::mat4> Skeleton::ComputeInverseBindMatrices() const {
  ASSIGN_CHECKED((std::vector<glm::mat4> matrices),
                 ComputePoseMatrices(GetBindPose()));
  for (glm::mat4& matrix : matrices) {
    matrix = glm::inverse(matrix);
  }
  return matrices;
}
