
#include "resources/shader.h"

#include <GL/GL.h>
#include <GL/glew.h>
#include <glog/logging.h>

#include <fstream>
#include <optional>
#include <sstream>

#include "resources/resource.h"

absl::StatusOr<std::string> getShaderCode(const Shader::Details& details) {
  if (!details.read_file) {
    return details.source;
  }
  std::ifstream shader_file;
  shader_file.open(details.source);
  if (!shader_file.is_open()) {
    return absl::NotFoundError(
        STATUS_MESSAGE("Failed to read file \"" << details.source << "\""));
  }
  std::stringstream contents;
  contents << shader_file.rdbuf();
  shader_file.close();
  return contents.str();
}

GLuint getGLShaderType(const Shader::Type type) {
  switch (type) {
    case Shader::Type::Vertex:
      return GL_VERTEX_SHADER;
    case Shader::Type::Fragment:
      return GL_FRAGMENT_SHADER;
    default:
      CHECK(false);
      return 0;
  }
}

absl::StatusOr<std::shared_ptr<Shader>> Shader::Load(const Details& details) {
  std::shared_ptr<Shader> shader(new Shader());
  ASSIGN_OR_RETURN(const std::string& shader_code, getShaderCode(details));

  const char* shader_source = shader_code.c_str();

  shader->id = glCreateShader(getGLShaderType(details.type));
  glShaderSource(shader->id, 1, &shader_source, NULL);
  glCompileShader(shader->id);
  GLint success;
  glGetShaderiv(shader->id, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLint info_length;
    glGetShaderiv(shader->id, GL_INFO_LOG_LENGTH, &info_length);
    std::string log;
    log.resize(info_length + 1);
    log.back() = 0;
    glGetShaderInfoLog(shader->id, info_length, NULL, log.data());
    return absl::InvalidArgumentError(
        STATUS_MESSAGE("Failed to compile shader: " << log));
  }
  return shader;
}

Shader::~Shader() { glDeleteShader(id); }

absl::StatusOr<std::shared_ptr<Program>> Program::Load(const Details& details) {
  std::vector<std::shared_ptr<Shader>> shaders;
  shaders.reserve(details.vertex_shaders.size() +
                  details.fragment_shaders.size());
  for (const std::string& vertex_shader_name : details.vertex_shaders) {
    shaders.push_back(nullptr);
    ASSIGN_OR_RETURN(shaders.back(),
                     ResourceLoader::Get().Load<Shader>(vertex_shader_name));
    if (!shaders.back()) {
      std::cerr << "null shader?>??" << std::endl;
    }
  }
  for (const std::string& fragment_shader_name : details.fragment_shaders) {
    shaders.push_back(nullptr);
    ASSIGN_OR_RETURN(shaders.back(),
                     ResourceLoader::Get().Load<Shader>(fragment_shader_name));
  }
  std::shared_ptr<Program> program(new Program());
  program->id = glCreateProgram();
  for (const std::shared_ptr<Shader>& shader : shaders) {
    glAttachShader(program->id, shader->id);
  }
  glLinkProgram(program->id);

  GLint success;
  glGetProgramiv(program->id, GL_LINK_STATUS, &success);
  if (!success) {
    GLint info_length;
    glGetProgramiv(program->id, GL_INFO_LOG_LENGTH, &info_length);
    std::string log;
    log.resize(info_length + 1);
    log.back() = 0;
    glGetProgramInfoLog(program->id, info_length, NULL, log.data());
    return absl::InvalidArgumentError(
        STATUS_MESSAGE("Failed to link program: " << log));
  }

  for (const std::shared_ptr<Shader>& shader : shaders) {
    glDetachShader(program->id, shader->id);
  }

  return program;
}

Program::~Program() { glDeleteProgram(id); }

void Program::Use() { glUseProgram(id); }

GLuint Program::GetUniformLocation(const std::string& name) const {
  return glGetUniformLocation(id, name.c_str());
}
