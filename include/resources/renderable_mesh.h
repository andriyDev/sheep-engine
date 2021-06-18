
#pragma once

#include <GL/glew.h>
#include <absl/status/statusor.h>

#include <memory>
#include <optional>

#include "resources/mesh.h"
#include "resources/skin.h"
#include "utility/resource_handle.h"

class RenderableMesh {
 public:
  struct Details {
    ResourceHandle<Mesh> mesh;
    std::optional<ResourceHandle<Skin>> skin;
  };
  using detail_type = Details;

  static absl::StatusOr<std::shared_ptr<RenderableMesh>> Load(
      const Details& details);

  virtual ~RenderableMesh();

  void Draw();

 private:
  enum class Indexing { None, Small, Large };

  std::vector<GLuint> buffers;
  GLuint vao = 0;
  unsigned int elements;
  unsigned int vertex_attribute_count;
  Indexing indexing = Indexing::None;
};
