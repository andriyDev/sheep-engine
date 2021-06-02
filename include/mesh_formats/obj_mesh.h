
#pragma once

#include <absl/container/flat_hash_map.h>

#include <memory>
#include <string>

#include "mesh.h"

struct ObjModel {
  struct Details {
    std::string file;
  };
  using detail_type = Details;

  static std::shared_ptr<ObjModel> Load(const Details& details);

  struct MeshDetails {
    std::string obj_model;
    std::string name;
  };
  static std::shared_ptr<Mesh> LoadMesh(const MeshDetails& details);

  absl::flat_hash_map<std::string, std::shared_ptr<Mesh>> meshes;
};
