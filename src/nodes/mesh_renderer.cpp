
#include "nodes/mesh_renderer.h"

#include <glm/gtx/string_cast.hpp>

void MeshRenderer::Render(
    const std::shared_ptr<RenderSuperSystem>& super_system,
    const std::shared_ptr<RenderSystem>& system,
    const glm::mat4& ProjectionView) {
  for (const MeshInfo& mesh_info : meshes) {
    if (!mesh_info.mesh || !mesh_info.material) {
      continue;
    }
    mesh_info.material->Use();
    const glm::mat4 mvp = ProjectionView * GetGlobalMatrix();
    glUniformMatrix4fv(mesh_info.material->GetUniformLocation("MVP"), 1, false,
                       &mvp[0][0]);
    mesh_info.mesh->Draw();
  }
}
