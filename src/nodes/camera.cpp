
#include "nodes/camera.h"

#include <glm/gtc/matrix_inverse.hpp>

glm::mat4 Camera::GetProjectionMatrix(float aspect) const {
  switch (projection_type) {
    case Projection::Perspective:
      return glm::perspective(fov * 3.14159f / 180.f, aspect, near, far);
    case Projection::Orthographic:
      return glm::ortho(-0.5 * size * aspect, 0.5 * size * aspect, 0.5 * size,
                        -0.5 * size);
  }
  assert(false);
  return glm::mat4();
}

glm::mat4 Camera::GetProjectionView(float aspect) const {
  return GetProjectionMatrix(aspect) * glm::affineInverse(GetGlobalMatrix());
}
