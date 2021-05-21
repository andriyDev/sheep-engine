
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

#include <memory>
#include <string>

#include "resource.h"
#include "shader.h"

struct Mesh {
  GLuint vao;
  GLuint vbo;
};

Mesh triangleVao() {
  Mesh mesh;
  glGenVertexArrays(1, &mesh.vao);
  glBindVertexArray(mesh.vao);

  static const GLfloat data[] = {
      -1.f, -1.f, 0.f, 1.f, -1.f, 0.f, 0.f, 1.f, 0.f,
  };

  glGenBuffers(1, &mesh.vbo);
  glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

  return mesh;
}

#define VERTEX_SHADER                        \
  "#version 330 core\n"                      \
  "layout(location = 0) in vec3 position;\n" \
  "void main() {\n"                          \
  "  gl_Position.xyz = position;\n"          \
  "  gl_Position.w = 1.0;\n"                 \
  "}\n"
#define FRAGMENT_SHADER      \
  "#version 330 core\n"      \
  "out vec3 color;\n"        \
  "void main() {\n"          \
  "  color = vec3(1,0,1);\n" \
  "}\n"

void initResources() {
  ResourceLoader::Get().Add<Shader>(
      "main_shader_vertex",
      Shader::Details{VERTEX_SHADER, false, Shader::Type::Vertex});
  ResourceLoader::Get().Add<Shader>(
      "main_shader_fragment",
      Shader::Details{FRAGMENT_SHADER, false, Shader::Type::Fragment});

  ResourceLoader::Get().Add<Program>(
      "main_program",
      Program::Details{{"main_shader_vertex"}, {"main_shader_fragment"}});
}

void glfw_error(int error, const char* description) {
  fprintf(stderr, "GLFW Error: %s\n", description);
}

int main() {
  if (!glfwInit()) {
    fprintf(stderr, "Error initializing GLFW\n");
    return 1;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  GLFWwindow* window = glfwCreateWindow(1280, 720, "Title", NULL, NULL);
  if (!window) {
    fprintf(stderr, "Failed to create window.\n");
    return 1;
  }
  glfwMakeContextCurrent(window);

  const GLenum err = glewInit();
  if (err != GLEW_OK) {
    fprintf(stderr, "Error initializing GLEW : %s\n", glewGetErrorString(err));
    return 1;
  }

  initResources();

  std::shared_ptr<Program> material =
      ResourceLoader::Get().Load<Program>("main_program");
  if (!material) {
    return 1;
  }
  Mesh mesh = triangleVao();

  while (!glfwWindowShouldClose(window)) {
    material->Use();
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray(0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteBuffers(1, &mesh.vbo);
  glDeleteVertexArrays(1, &mesh.vao);

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
