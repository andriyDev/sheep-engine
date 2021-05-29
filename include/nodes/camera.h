
#pragma once

#include "nodes/transform.h"

class Camera : public Transform {
 public:
  enum class Projection {
    Perspective,
    Orthographic,
  };
  enum ClearFlags {
    Depth = 1U,
    Colour = 2U,
  };

  // Whether to render this camera or not.
  bool render = true;
  // Defines which buffers should be cleared upon rendering.
  uint8_t clear_flags = ClearFlags::Depth | ClearFlags::Colour;
  // Determines the top left and bottom right of where to render on the screen
  // in normalized (0-1) coordinates.
  glm::vec2 viewport[2] = {glm::vec2(0, 0), glm::vec2(1, 1)};
  // Determines the ordering of rendering cameras, where the larger the value,
  // the later it is rendered.
  int sort_order = 0;

  // Field of view of the camera, for perspective projection.
  float fov = 90;
  // Size of the height in units, for orthographic projection.
  float size = 1;
  // Near plane distance.
  float near = 0.1f;
  // Far plane distance.
  float far = 5000.f;
  // Projection type to use.
  Projection projection_type = Projection::Perspective;

  // Computes the projection matrix of this camera given the view surface aspect
  // ratio.
  glm::mat4 GetProjectionMatrix(float aspect) const;

  // Computes the inverted view and projection matrices given the view surface
  // aspect ratio.
  glm::mat4 GetProjectionView(float aspect) const;
};
