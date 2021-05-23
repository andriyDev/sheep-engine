
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <string>

#include "mesh.h"
#include "mesh_formats/obj_mesh.h"
#include "resource.h"
#include "shader.h"

std::shared_ptr<Mesh> triangleMesh() {
  std::shared_ptr<Mesh> source_mesh(new Mesh());
  source_mesh->vertices = {{glm::vec3(-1.f, -1.f, 0.f)},
                           {glm::vec3(1.f, -1.f, 0.f)},
                           {glm::vec3(0.f, 1.f, 0.f)}};
  source_mesh->triangles = {{0, 1, 2}};
  return source_mesh;
}

#define VERTEX_SHADER                            \
  "#version 330 core\n"                          \
  "layout(location = 0) in vec3 position;\n"     \
  "layout(location = 2) in vec3 normal;\n"       \
  "uniform mat4 MVP;\n"                          \
  "out vec3 normal_frag;\n"                      \
  "void main() {\n"                              \
  "  gl_Position = MVP * vec4(position, 1.0);\n" \
  "  normal_frag = normal;\n"                    \
  "}\n"
#define FRAGMENT_SHADER                                                        \
  "#version 330 core\n"                                                        \
  "in vec3 normal_frag;\n"                                                     \
  "out vec3 color;\n"                                                          \
  "void main() {\n"                                                            \
  "  vec3 sun_dir = normalize(vec3(1,1,1));\n"                                 \
  "  color = vec3(1,1,1) * clamp(dot(normal_frag, sun_dir) * 0.5 + 0.5, 0.0, " \
  "1.0);\n"                                                                    \
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

  ResourceLoader::Get().Add<Mesh>("triangle_mesh", triangleMesh);
  ResourceLoader::Get().Add<RenderableMesh>("triangle_rmesh",
                                            {"triangle_mesh"});

  ResourceLoader::Get().Add<ObjModel>("obj", {"test_mesh.obj"});
  ResourceLoader::Get().Add<Mesh>("obj_mesh", ObjModel::LoadMesh,
                                  {"obj", "Cube"});
  ResourceLoader::Get().Add<RenderableMesh>("obj_rmesh", {"obj_mesh"});
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
  std::shared_ptr<RenderableMesh> mesh =
      ResourceLoader::Get().Load<RenderableMesh>("obj_rmesh");

  GLuint mvp_location = material->GetUniformLocation("MVP");

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.f, 0.f, 0.4f, 0.f);

  glfwSwapInterval(0);

  double previous_time = glfwGetTime();

  while (!glfwWindowShouldClose(window)) {
    double time = glfwGetTime();
    double delta = time - previous_time;
    previous_time = time;

    printf("Delta: %f\n", 1.0f / delta);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    material->Use();

    glm::mat4 mvp =
        glm::perspective(glm::radians(90.0f), 1280.f / 720.f, 0.1f, 100.f) *
        glm::lookAt(glm::quat(glm::vec3(0, time, 0)) * glm::vec3(0, 2, 3),
                    glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    glUniformMatrix4fv(mvp_location, 1, GL_FALSE, &mvp[0][0]);
    mesh->Draw();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
