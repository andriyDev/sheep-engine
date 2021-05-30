
#include "nodes/mesh_renderer.h"

#include <glm/gtx/string_cast.hpp>

void MeshRenderer::Render(
    const std::shared_ptr<RenderSuperSystem>& super_system,
    const std::shared_ptr<RenderSystem>& system,
    const glm::mat4& ProjectionView) {
  if (!material || !mesh) {
    return;
  }
  material->Use();
  const glm::mat4 mvp = ProjectionView * GetGlobalMatrix();
  // printf("%s\n", glm::to_string(mvp * glm::vec4(-1, 1, 5, 1)).c_str());
  glUniformMatrix4fv(material->GetUniformLocation("MVP"), 1, false, &mvp[0][0]);
  mesh->Draw();
}
