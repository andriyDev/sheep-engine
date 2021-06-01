
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
#include "systems/input_system.h"
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

glm::vec3 ToEuler(glm::quat q) {
  glm::vec3 f = q * glm::vec3(0, 0, -1);
  glm::vec3 r = q * glm::vec3(1, 0, 0);
  float pitch = asin(f.y);
  f.y = 0;
  f = glm::normalize(f);
  float yaw = atan2f(-f.x, -f.z);
  glm::vec3 p(-f.z, 0, f.x);
  // float roll = acos(glm::dot(r, p)) * (float(signbit(r.y)) * 2 - 1);
  return glm::vec3(yaw, pitch, 0) * 180.f / 3.14159f;
}

glm::quat FromEuler(glm::vec3 e) {
  e *= 3.14159f / 180.f;
  return glm::angleAxis(e.x, glm::vec3(0, 1, 0)) *
         glm::angleAxis(e.y, glm::vec3(1, 0, 0)) *
         glm::angleAxis(e.z, glm::vec3(0, 0, -1));
}

float clamp(float value, float m, float M) {
  return std::max(std::min(value, M), m);
}

class PlayerControlSystem : public System {
 public:
  // Moves around the parent node based on player input.
  class PlayerNode : public Node {
   public:
    float look_sensitivity = 0.1f;
    float move_speed = 1.f;
  };

  void Init() override {
    input_system_weak = GetEngine()->GetSuperSystem<InputSuperSystem>();
  }

  void Update(float delta_seconds) override {
    std::shared_ptr<InputSuperSystem> input_system = input_system_weak.lock();
    glm::vec2 move(input_system->GetAxisValue("player/move/horizontal"),
                   input_system->GetAxisValue("player/move/vertical"));
    glm::vec3 look(-input_system->GetAxisValue("player/look/horizontal"),
                   -input_system->GetAxisValue("player/look/vertical"),
                   input_system->GetAxisValue("player/look/roll"));
    for (const std::shared_ptr<PlayerNode>& node : player_nodes) {
      const std::shared_ptr<Transform> node_transform =
          Transform::GetFirstTransform(node->GetParent());
      if (!node_transform) {
        continue;
      }
      glm::vec3 euler = ToEuler(node_transform->GetRotation());
      euler.x += look.x * node->look_sensitivity;
      euler.y = clamp(euler.y + look.y * node->look_sensitivity, -89, 89);
      euler.z += look.z * delta_seconds;
      node_transform->SetRotation(FromEuler(euler));
      // printf("Euler: %s\n", glm::to_string(euler).c_str());
      glm::vec3 forward = node_transform->GetRotation() * glm::vec3(0, 0, -1);
      glm::vec3 right = node_transform->GetRotation() * glm::vec3(1, 0, 0);
      node_transform->SetPosition(node_transform->GetPosition() +
                                  forward * move.y + right * move.x);
    }
  }

  void NotifyOfNodeAttachment(const std::shared_ptr<Node>& root) override {
    player_nodes.AddTree(root);
  }
  void NotifyOfNodeDetachment(const std::shared_ptr<Node>& root) override {
    player_nodes.RemoveTree(root);
  }

 private:
  std::weak_ptr<InputSuperSystem> input_system_weak;
  NodeTypeGroup<PlayerNode> player_nodes;
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
  {
    auto input_system = std::static_pointer_cast<InputSuperSystem>(
        engine->AddSuperSystem(std::make_shared<InputSuperSystem>(window)));
    input_system->CreateAxis(
        "player/move/horizontal",
        {InputSuperSystem::AxisDefinition::Key(GLFW_KEY_D, 1.f),
         InputSuperSystem::AxisDefinition::Key(GLFW_KEY_A, -1.f)});
    input_system->CreateAxis(
        "player/move/vertical",
        {InputSuperSystem::AxisDefinition::Key(GLFW_KEY_W, 1.f),
         InputSuperSystem::AxisDefinition::Key(GLFW_KEY_S, -1.f)});
    input_system->CreateAxis(
        "player/look/horizontal",
        {InputSuperSystem::AxisDefinition::MouseMove(
            InputSuperSystem::AxisDefinition::Direction::Horizontal, 1.f)});
    input_system->CreateAxis(
        "player/look/vertical",
        {InputSuperSystem::AxisDefinition::MouseMove(
            InputSuperSystem::AxisDefinition::Direction::Vertical, 1.f)});
    input_system->CreateAxis(
        "player/look/roll",
        {InputSuperSystem::AxisDefinition::Key(GLFW_KEY_E, 1.f),
         InputSuperSystem::AxisDefinition::Key(GLFW_KEY_Q, -1.f)});
  }
  std::shared_ptr<World> world = engine->CreateWorld();
  world->CreateEmptyRoot();
  world->AddSystem(std::make_shared<PlayerControlSystem>());

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
    camera->AttachTo(camera_pivot);
    camera_pivot->AttachTo(world->GetRoot());

    std::shared_ptr<PlayerControlSystem::PlayerNode>(
        new PlayerControlSystem::PlayerNode())
        ->AttachTo(camera);
  }

  engine->Run(window);

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
