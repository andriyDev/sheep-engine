
#include "nodes/skinned_mesh_renderer.h"

SkinnedMeshRenderer::~SkinnedMeshRenderer() {
  glDeleteBuffers(1, &pose_buffer);
}

void SkinnedMeshRenderer::Render(
    const std::shared_ptr<RenderSuperSystem>& super_system,
    const std::shared_ptr<RenderSystem>& system,
    const glm::mat4& ProjectionView) {
  if (!skeleton) {
    return;
  }
  if (rebuild_pose_buffer) {
    if (!pose_buffer) {
      glCreateBuffers(1, &pose_buffer);
      glBindBuffer(GL_UNIFORM_BUFFER, pose_buffer);
    }
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * skeleton->bones.size(),
                 NULL, GL_DYNAMIC_DRAW);
    rebuild_pose_buffer = false;
  }

  glBindBuffer(GL_UNIFORM_BUFFER, pose_buffer);
  ASSIGN_CHECKED(
      (const std::vector<glm::mat4>& pose_matrices),
      skeleton->ComputeRelativePoseMatrices(skeleton->GetBindPose()));
  glBufferSubData(GL_UNIFORM_BUFFER, 0,
                  sizeof(glm::mat4) * skeleton->bones.size(),
                  pose_matrices.data());
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, pose_buffer);

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
  if (skeleton == new_skeleton) {
    return;
  }
  if (!skeleton || !new_skeleton ||
      skeleton->bones.size() != new_skeleton->bones.size()) {
    rebuild_pose_buffer = true;
  }
  skeleton = new_skeleton;
}

const std::shared_ptr<Skeleton>& SkinnedMeshRenderer::GetSkeleton() const {
  return skeleton;
}
