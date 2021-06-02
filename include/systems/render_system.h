
#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "nodes/camera.h"
#include "nodes/node.h"
#include "systems/super_system.h"
#include "systems/system.h"
#include "utility/type_group.h"

class RenderSystem;
class RenderSuperSystem;

class Renderable {
 protected:
  virtual void Render(const std::shared_ptr<RenderSuperSystem>& super_system,
                      const std::shared_ptr<RenderSystem>& system,
                      const glm::mat4& ProjectionView) = 0;

  friend class RenderSuperSystem;
};

class RenderSystem : public System {
 protected:
  void NotifyOfNodeAttachment(const std::shared_ptr<Node>& new_node) override;
  void NotifyOfNodeDetachment(const std::shared_ptr<Node>& new_node) override;

 private:
  NodeTypeGroup<Renderable> renderables;
  NodeTypeGroup<Camera> cameras;

  friend class RenderSuperSystem;
};

class RenderSuperSystem : public SuperSystem {
 public:
  RenderSuperSystem(GLFWwindow* window_);

  enum class RenderSystemAddition {
    None,        // RenderSystems must be manually attached to all worlds.
    InitWorlds,  // RenderSystems will only be added to worlds present on
                 // initialization.
    AllWorlds,   // RenderSystems will be added to all worlds as they are
                 // initialized.
  };

  RenderSystemAddition addition_mode = RenderSystemAddition::AllWorlds;

 protected:
  void Init() override;

  void LateUpdate(float delta_seconds) override;

  void NotifyOfWorldInitialization(
      const std::shared_ptr<World>& world) override;
  void NotifyOfSystemAddition(const std::shared_ptr<World>& world,
                              const std::shared_ptr<System>& system) override;
  void NotifyOfSystemRemoval(const std::shared_ptr<World>& world,
                             const std::shared_ptr<System>& system) override;

 private:
  SystemTypeGroup<RenderSystem> render_systems;
  GLFWwindow* window;
};
