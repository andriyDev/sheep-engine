
#pragma once

#include <GL/glew.h>

#include <memory>
#include <string>
#include <vector>

class Shader {
 public:
  enum class Type { Vertex, Fragment };

  struct Details {
    std::string source;
    bool read_file;
    Type type;
  };
  using detail_type = Details;

  static std::shared_ptr<Shader> Load(const Details& details);

  virtual ~Shader();

 private:
  GLuint id = 0;

  friend class Program;
};

class Program {
 public:
  struct Details {
    std::vector<std::string> vertex_shaders;
    std::vector<std::string> fragment_shaders;
  };
  using detail_type = Details;

  static std::shared_ptr<Program> Load(const Details& details);

  virtual ~Program();

  void Use();

  GLuint GetUniformLocation(const std::string& name) const;

 private:
  GLuint id = 0;
};
