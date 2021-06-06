
#pragma once

#include <GL/glew.h>
#include <absl/status/statusor.h>

#include <memory>
#include <string>
#include <vector>

#include "utility/resource_handle.h"

class Shader {
 public:
  enum class Type { Vertex, Fragment };

  struct Details {
    std::string source;
    bool read_file;
    Type type;
  };
  using detail_type = Details;

  static absl::StatusOr<std::shared_ptr<Shader>> Load(const Details& details);

  virtual ~Shader();

 private:
  GLuint id = 0;

  friend class Program;
};

class Program {
 public:
  struct Details {
    std::vector<ResourceHandle<Shader>> vertex_shaders;
    std::vector<ResourceHandle<Shader>> fragment_shaders;
  };
  using detail_type = Details;

  static absl::StatusOr<std::shared_ptr<Program>> Load(const Details& details);

  virtual ~Program();

  void Use();

  GLuint GetUniformLocation(const std::string& name) const;

 private:
  GLuint id = 0;
};
