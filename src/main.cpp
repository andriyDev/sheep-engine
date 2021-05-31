
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <memory>
#include <sstream>
#include <string>

#include "engine.h"
#include "mesh.h"
#include "mesh_formats/obj_mesh.h"
#include "nodes/mesh_renderer.h"
#include "nodes/transform.h"
#include "resource.h"
#include "shader.h"
#include "systems/render_system.h"
#include "utility/cached.h"

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
                                  {"obj", "Blob"});
  ResourceLoader::Get().Add<RenderableMesh>("obj_rmesh", {"obj_mesh"});
}

class SpinSystem : public System {
 public:
  // Spins the parent node.
  class SpinNode : public Node {
   public:
    float degreesRatePerSecond = 45;
  };

  void Update(float delta_seconds) override {
    for (const std::shared_ptr<SpinNode>& node : spin_nodes) {
      const std::shared_ptr<Transform> node_transform =
          Transform::GetFirstTransform(node->GetParent());
      if (!node_transform) {
        continue;
      }
      node_transform->SetRotation(
          node_transform->GetRotation() *
          glm::angleAxis(
              node->degreesRatePerSecond * 3.14159f / 180 * delta_seconds,
              glm::vec3(0, 1, 0)));
    }
  }

  void NotifyOfNodeAttachment(const std::shared_ptr<Node>& root) override {
    spin_nodes.AddTree(root);
  }
  void NotifyOfNodeDetachment(const std::shared_ptr<Node>& root) override {
    spin_nodes.RemoveTree(root);
  }

 private:
  NodeTypeGroup<SpinNode> spin_nodes;
};

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

  std::shared_ptr<Engine> engine(new Engine());
  engine->AddSuperSystem(std::make_shared<RenderSuperSystem>(window));
  std::shared_ptr<World> world = engine->CreateWorld();
  world->CreateEmptyRoot();
  world->AddSystem(std::make_shared<SpinSystem>());
  world->AddSystem(std::make_shared<RenderSystem>());

  std::shared_ptr<Transform> camera_pivot;
  std::shared_ptr<Camera> camera;
  {
    std::shared_ptr<MeshRenderer> mesh_renderer(new MeshRenderer());
    mesh_renderer->AttachTo(world->GetRoot());

    std::shared_ptr<Program> material =
        ResourceLoader::Get().Load<Program>("main_program");
    if (!material) {
      return 1;
    }
    std::shared_ptr<RenderableMesh> mesh =
        ResourceLoader::Get().Load<RenderableMesh>("obj_rmesh");
    if (!mesh) {
      return 1;
    }
    mesh_renderer->mesh = mesh;
    mesh_renderer->material = material;

    camera_pivot = std::shared_ptr<Transform>(new Transform());
    camera = std::shared_ptr<Camera>(new Camera());
    camera->SetPosition(glm::vec3(0, 0, 5));
    camera->viewport[0] = glm::vec2(0, 0.5);
    camera->viewport[1] = glm::vec2(0.5, 1);
    camera->AttachTo(camera_pivot);
    camera_pivot->AttachTo(world->GetRoot());

    std::shared_ptr<SpinSystem::SpinNode>(new SpinSystem::SpinNode())
        ->AttachTo(camera_pivot);
  }

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.f, 0.f, 0.4f, 0.f);

  glfwSwapInterval(0);

  engine->Run(window);

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
