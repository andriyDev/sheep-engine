
#include "systems/render_system.h"

#include <GL/glew.h>

#include <algorithm>

#include "engine.h"
#include "nodes/utility.h"

void RenderSystem::NotifyOfNodeAttachment(
    const std::shared_ptr<Node>& new_node) {
  const std::vector<std::shared_ptr<Node>> nodes =
      CollectPreOrderNodes(new_node);
  renderables.Add(nodes);
  cameras.Add(nodes);
}

void RenderSystem::NotifyOfNodeDetachment(
    const std::shared_ptr<Node>& new_node) {
  const std::vector<std::shared_ptr<Node>> nodes =
      CollectPreOrderNodes(new_node);
  renderables.Remove(nodes);
  cameras.Remove(nodes);
}

RenderSuperSystem::RenderSuperSystem(GLFWwindow* window_) : window(window_) {}

void RenderSuperSystem::LateUpdate(float delta_seconds) {
  std::vector<std::pair<std::shared_ptr<RenderSystem>, std::shared_ptr<Camera>>>
      ordered_cameras;
  for (const std::shared_ptr<RenderSystem>& render_system : render_systems) {
    for (const std::shared_ptr<Camera>& camera : render_system->cameras) {
      if (camera->render) {
        ordered_cameras.push_back(std::make_pair(render_system, camera));
      }
    }
  }

  std::sort(ordered_cameras.begin(), ordered_cameras.end(),
            [](const std::pair<std::shared_ptr<RenderSystem>,
                               std::shared_ptr<Camera>>& a,
               const std::pair<std::shared_ptr<RenderSystem>,
                               std::shared_ptr<Camera>>& b) {
              return a.second->sort_order - b.second->sort_order;
            });

  int width, height;
  glfwGetWindowSize(window, &width, &height);

  for (const auto& [render_system, camera] : ordered_cameras) {
    int x1 = (int)ceil(width * camera->viewport[0].x),
        y1 = (int)ceil(height * camera->viewport[0].y),
        x2 = (int)ceil(width * camera->viewport[1].x),
        y2 = (int)ceil(height * camera->viewport[1].y);

    glViewport(x1, y1, x2 - x1, y2 - y1);

    bool clear_depth = bool(camera->clear_flags & (Camera::ClearFlags::Depth));
    bool clear_colour =
        bool(camera->clear_flags & (Camera::ClearFlags::Colour));
    glClear((GL_COLOR_BUFFER_BIT * clear_colour) |
            (GL_DEPTH_BUFFER_BIT * clear_depth));
    const glm::mat4 pv =
        camera->GetProjectionView((float)(x2 - x1) / (float)(y2 - y1));
    for (const std::shared_ptr<Renderable>& renderable :
         render_system->renderables) {
      renderable->Render(
          std::static_pointer_cast<RenderSuperSystem>(this->shared_from_this()),
          render_system, pv);
    }
  }
  glfwSwapBuffers(window);
}

void RenderSuperSystem::NotifyOfSystemAddition(
    const std::shared_ptr<World>& world,
    const std::shared_ptr<System>& system) {
  render_systems.AddSystem(system);
}

void RenderSuperSystem::NotifyOfSystemRemoval(
    const std::shared_ptr<World>& world,
    const std::shared_ptr<System>& system) {
  render_systems.AddSystem(system);
}
