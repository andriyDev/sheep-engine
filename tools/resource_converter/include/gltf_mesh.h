
#pragma once

#include <absl/container/flat_hash_map.h>

#include <memory>
#include <string>
#include <vector>

#include "resources/mesh.h"
#include "resources/skin.h"
#include "utility/status.h"

struct GltfModel {
 public:
  struct Primitive {
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Skin> skin;
  };
  absl::flat_hash_map<std::string, std::shared_ptr<Skeleton>> skeletons;
  absl::flat_hash_map<std::string, std::vector<Primitive>> primitives;

  static absl::StatusOr<GltfModel> Load(const std::string& filename);
};
