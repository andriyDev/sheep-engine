
#pragma once

#include <memory>

#include "mesh.h"
#include "nodes/transform.h"
#include "shader.h"
#include "systems/render_system.h"

class MeshRenderer : public Transform, public Renderable {
 public:
  std::shared_ptr<RenderableMesh> mesh;
  std::shared_ptr<Program> material;

 protected:
  void Render(const std::shared_ptr<RenderSystem>& system,
              const glm::mat4& ProjectionView) override;
};
