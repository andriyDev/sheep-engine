
#pragma once

#include <memory>
#include <vector>

#include "nodes/transform.h"
#include "resources/shader.h"
#include "resources/skeleton.h"
#include "resources/skinned_mesh.h"
#include "systems/render_system.h"

class SkinnedMeshRenderer : public Transform, public Renderable {
 public:
  struct MeshInfo {
    std::shared_ptr<SkinnedMesh> mesh;
    std::shared_ptr<Program> material;
  };
  std::vector<MeshInfo> meshes;

  virtual ~SkinnedMeshRenderer();

  void SetSkeleton(const std::shared_ptr<Skeleton>& new_skeleton);
  const std::shared_ptr<Skeleton>& GetSkeleton() const;

 protected:
  void Render(const std::shared_ptr<RenderSuperSystem>& super_system,
              const std::shared_ptr<RenderSystem>& system,
              const glm::mat4& ProjectionView) override;

  std::shared_ptr<Skeleton> skeleton;
  GLuint pose_buffer = 0;
  bool rebuild_pose_buffer = true;
};
