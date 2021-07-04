
#include "nodes/skinned_mesh_renderer.h"

void SkinnedMeshRenderer::Render(
    const std::shared_ptr<RenderSuperSystem>& super_system,
    const std::shared_ptr<RenderSystem>& system,
    const glm::mat4& ProjectionView) {
  if (!material || !mesh) {
    return;
  }
  material->Use();
  const glm::mat4 mvp = ProjectionView * GetGlobalMatrix();
  glUniformMatrix4fv(material->GetUniformLocation("MVP"), 1, false, &mvp[0][0]);
  mesh->DrawSkinned();
}
