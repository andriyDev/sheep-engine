
#pragma once

#include "resources/renderable_mesh.h"

class SkinnedMesh : public RenderableMesh {
 public:
  struct Details {
    ResourceHandle<Mesh> mesh;
    ResourceHandle<Skin> skin;
  };
  using detail_type = Details;

  static absl::StatusOr<std::shared_ptr<SkinnedMesh>> Load(
      const Details& details);

  void DrawSkinned();

  std::shared_ptr<Skeleton> GetSkeleton() const;

 private:
  std::shared_ptr<Skeleton> skeleton;
};
