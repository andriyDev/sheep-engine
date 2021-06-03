
#pragma once

#include <memory>

#include "nodes/transform.h"
#include "resources/mesh.h"
#include "resources/shader.h"
#include "systems/render_system.h"

class MeshRenderer : public Transform, public Renderable {
 public:
  std::shared_ptr<RenderableMesh> mesh;
  std::shared_ptr<Program> material;

 protected:
  void Render(const std::shared_ptr<RenderSuperSystem>& super_system,
              const std::shared_ptr<RenderSystem>& system,
              const glm::mat4& ProjectionView) override;
};
