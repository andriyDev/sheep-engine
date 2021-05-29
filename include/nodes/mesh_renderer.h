
#pragma once

#include <memory>

#include "mesh.h"
#include "nodes/transform.h"
#include "shader.h"

class MeshRenderer : public Transform {
 public:
  std::shared_ptr<RenderableMesh> mesh;
  std::shared_ptr<Program> material;
};
