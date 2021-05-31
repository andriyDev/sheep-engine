
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "nodes/node.h"
#include "utility/cached.h"

class Transform : public Node {
 public:
  Transform();

  glm::vec3 GetPosition() const;
  glm::quat GetRotation() const;
  glm::vec3 GetScale() const;
  glm::mat4 GetMatrix() const;

  void SetPosition(const glm::vec3& value);
  void SetRotation(const glm::quat& value);
  void SetScale(const glm::vec3& value);

  glm::vec3 GetGlobalPosition() const;
  glm::quat GetGlobalRotation() const;
  glm::vec3 GetLossyScale() const;
  glm::mat4 GetGlobalMatrix() const;

  void SetGlobalPosition(const glm::vec3& value);
  void SetGlobalRotation(const glm::quat& value);

  std::shared_ptr<Transform> GetParentTransform() const;

  static std::shared_ptr<Transform> GetFirstTransform(
      const std::shared_ptr<Node>& leaf);

 protected:
  void AttachNode(const std::shared_ptr<Node>& child, int index) override;
  void DetachNode(const std::shared_ptr<Node>& child) override;

  void NotifyOfAncestorAttachment(
      const std::shared_ptr<Node>& new_parent,
      const std::shared_ptr<Node>& root_ancestor) override;
  void NotifyOfAncestorDetachment(
      const std::shared_ptr<Node>& parent,
      const std::shared_ptr<Node>& root_ancestor) override;

 private:
  glm::vec3 position = glm::vec3(0, 0, 0);
  glm::quat rotation = glm::quat(1, 0, 0, 0);
  glm::vec3 scale = glm::vec3(1, 1, 1);
  Cached<glm::mat4> matrix;
  Cached<glm::mat4> global_matrix;
  Cached<glm::quat> global_rotation;

  static glm::mat4 ComputeMatrix(const Transform* transform);
  static glm::mat4 ComputeGlobalMatrix(const Transform* transform);
  static glm::quat ComputeGlobalRotation(const Transform* transform);
};
