
#include "nodes/transform.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

#include "nodes/utility.h"

glm::mat4 TRSMatrix(const glm::vec3& translation, const glm::quat& rotation,
                    const glm::vec3& scale) {
  return glm::translate(glm::identity<glm::mat4>(), translation) *
         glm::toMat4(rotation) * glm::scale(glm::identity<glm::mat4>(), scale);
}

Transform::Transform()
    : matrix([this]() { return ComputeMatrix(this); }),
      global_matrix([this]() { return ComputeGlobalMatrix(this); }),
      global_rotation([this]() { return ComputeGlobalRotation(this); }) {}

glm::vec3 Transform::GetPosition() const { return position; }
glm::quat Transform::GetRotation() const { return rotation; }
glm::vec3 Transform::GetScale() const { return scale; }
glm::mat4 Transform::GetMatrix() const { return *matrix; }

void Transform::SetPosition(const glm::vec3& value) {
  position = value;
  matrix.Invalidate();
  for (const std::shared_ptr<Node>& child :
       CollectPreOrderNodes(this->shared_from_this())) {
    const std::shared_ptr<Transform> child_transform =
        std::dynamic_pointer_cast<Transform>(child);
    if (!child_transform) {
      continue;
    }
    child_transform->global_matrix.Invalidate();
  }
}
void Transform::SetRotation(const glm::quat& value) {
  rotation = value;
  matrix.Invalidate();
  for (const std::shared_ptr<Node>& child :
       CollectPreOrderNodes(this->shared_from_this())) {
    const std::shared_ptr<Transform> child_transform =
        std::dynamic_pointer_cast<Transform>(child);
    if (!child_transform) {
      continue;
    }
    child_transform->global_matrix.Invalidate();
    child_transform->global_rotation.Invalidate();
  }
}
void Transform::SetScale(const glm::vec3& value) {
  scale = value;
  matrix.Invalidate();
  for (const std::shared_ptr<Node>& child :
       CollectPreOrderNodes(this->shared_from_this())) {
    const std::shared_ptr<Transform> child_transform =
        std::dynamic_pointer_cast<Transform>(child);
    if (!child_transform) {
      continue;
    }
    child_transform->global_matrix.Invalidate();
  }
}

glm::vec3 Transform::GetGlobalPosition() const {
  return glm::vec3((*global_matrix) * glm::vec4(0, 0, 0, 1));
}

glm::quat Transform::GetGlobalRotation() const { return *global_rotation; }

glm::vec3 Transform::GetLossyScale() const {
  const glm::mat4 near_scale =
      glm::inverse(TRSMatrix(GetGlobalPosition(), GetGlobalRotation(),
                             glm::vec3(1, 1, 1))) *
      (*global_matrix);
  return glm::vec3(near_scale[0][0], near_scale[1][1], near_scale[2][2]);
}

glm::mat4 Transform::GetGlobalMatrix() const { return *global_matrix; }

void Transform::SetGlobalPosition(const glm::vec3& value) {
  const std::shared_ptr<Transform> parent = this->GetParentTransform();
  if (parent) {
    position = glm::vec3(glm::inverse(parent->GetGlobalMatrix()) *
                         glm::vec4(value, 1.0));
  } else {
    position = value;
  }
  matrix.Invalidate();
  for (const std::shared_ptr<Node>& child :
       CollectPreOrderNodes(this->shared_from_this())) {
    const std::shared_ptr<Transform> child_transform =
        std::dynamic_pointer_cast<Transform>(child);
    if (!child_transform) {
      continue;
    }
    child_transform->global_matrix.Invalidate();
  }
}

void Transform::SetGlobalRotation(const glm::quat& value) {
  const std::shared_ptr<Transform> parent = this->GetParentTransform();
  if (parent) {
    rotation = glm::inverse(parent->GetGlobalRotation()) * value;
  } else {
    rotation = value;
  }
  matrix.Invalidate();
  for (const std::shared_ptr<Node>& child :
       CollectPreOrderNodes(this->shared_from_this())) {
    const std::shared_ptr<Transform> child_transform =
        std::dynamic_pointer_cast<Transform>(child);
    if (!child_transform) {
      continue;
    }
    child_transform->global_matrix.Invalidate();
    child_transform->global_rotation.Invalidate();
  }
}

std::shared_ptr<Transform> Transform::GetParentTransform() const {
  return GetFirstTransform(GetParent());
}

std::shared_ptr<Transform> Transform::GetFirstTransform(
    const std::shared_ptr<Node>& leaf) {
  std::shared_ptr<Node> node = leaf;
  while (node) {
    auto node_as_transform = std::dynamic_pointer_cast<Transform>(node);
    if (node_as_transform) {
      return node_as_transform;
    } else {
      node = node->GetParent();
    }
  }
  return nullptr;
}

void Transform::AttachNode(const std::shared_ptr<Node>& child, int index) {
  Node::AttachNode(child, index);
}

void Transform::DetachNode(const std::shared_ptr<Node>& child) {
  Node::DetachNode(child);
}

void Transform::NotifyOfAncestorAttachment(
    const std::shared_ptr<Node>& new_parent,
    const std::shared_ptr<Node>& root_ancestor) {
  global_matrix.Invalidate();
  global_rotation.Invalidate();
}

void Transform::NotifyOfAncestorDetachment(
    const std::shared_ptr<Node>& parent,
    const std::shared_ptr<Node>& root_ancestor) {
  global_matrix.Invalidate();
  global_rotation.Invalidate();
}

glm::mat4 Transform::ComputeMatrix(const Transform* transform) {
  return TRSMatrix(transform->position, transform->rotation, transform->scale);
}

glm::mat4 Transform::ComputeGlobalMatrix(const Transform* transform) {
  const std::shared_ptr<Transform> parent = transform->GetParentTransform();
  if (parent) {
    return parent->GetGlobalMatrix() * transform->GetMatrix();
  } else {
    return transform->GetMatrix();
  }
}

glm::quat Transform::ComputeGlobalRotation(const Transform* transform) {
  const std::shared_ptr<Transform> parent = transform->GetParentTransform();
  if (parent) {
    return parent->GetGlobalRotation() * transform->GetRotation();
  } else {
    return transform->GetRotation();
  }
}
