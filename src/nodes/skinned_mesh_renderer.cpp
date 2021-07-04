
#include "nodes/skinned_mesh_renderer.h"

void SkinnedMeshRenderer::Render(
    const std::shared_ptr<RenderSuperSystem>& super_system,
    const std::shared_ptr<RenderSystem>& system,
    const glm::mat4& ProjectionView) {
  if (!skeleton) {
    return;
  }
  for (const MeshInfo& mesh_info : meshes) {
    if (!mesh_info.mesh || !mesh_info.material ||
        mesh_info.mesh->GetSkeleton() != skeleton) {
      continue;
    }
    mesh_info.material->Use();
    const glm::mat4 mvp = ProjectionView * GetGlobalMatrix();
    glUniformMatrix4fv(mesh_info.material->GetUniformLocation("MVP"), 1, false,
                       &mvp[0][0]);
    mesh_info.mesh->DrawSkinned();
  }
}

void SkinnedMeshRenderer::SetSkeleton(
    const std::shared_ptr<Skeleton>& new_skeleton) {
  skeleton = new_skeleton;
}

const std::shared_ptr<Skeleton>& SkinnedMeshRenderer::GetSkeleton() const {
  return skeleton;
}
