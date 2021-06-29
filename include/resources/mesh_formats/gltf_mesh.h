
#pragma once

#include <absl/container/flat_hash_map.h>
#include <absl/status/statusor.h>

#include <memory>
#include <string>
#include <vector>

#include "resources/mesh.h"
#include "resources/skin.h"
#include "utility/resource_handle.h"

struct GltfModel {
  struct Details {
    std::string file;
  };
  using detail_type = Details;

  static absl::StatusOr<std::shared_ptr<GltfModel>> Load(
      const Details& details);

  struct PrimitiveDetails {
    ResourceHandle<GltfModel> model;
    std::string mesh_name;
    unsigned int index;
  };
  static absl::StatusOr<std::shared_ptr<Mesh>> LoadMesh(
      const PrimitiveDetails& details);

  static absl::StatusOr<std::shared_ptr<Skin>> LoadSkin(
      const PrimitiveDetails& details);

 private:
  struct Primitive {
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Skin> skin;
  };
  absl::flat_hash_map<std::string, std::vector<Primitive>> primitives;
};
