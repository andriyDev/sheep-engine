
#pragma once

#include "nodes/transform.h"
#include "resources/shader.h"
#include "resources/skinned_mesh.h"
#include "systems/render_system.h"

class SkinnedMeshRenderer : public Transform, public Renderable {
 public:
  std::shared_ptr<SkinnedMesh> mesh;
  std::shared_ptr<Program> material;

 protected:
  void Render(const std::shared_ptr<RenderSuperSystem>& super_system,
              const std::shared_ptr<RenderSystem>& system,
              const glm::mat4& ProjectionView) override;
};
