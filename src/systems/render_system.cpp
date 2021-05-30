
#include "systems/render_system.h"

#include <GL/glew.h>

#include <algorithm>

#include "nodes/utility.h"

void RenderSystem::NotifyOfNodeAttachment(
    const std::shared_ptr<Node>& new_node) {
  const std::vector<std::shared_ptr<Node>> nodes =
      CollectPreOrderNodes(new_node);
  for (const std::shared_ptr<Node>& node : nodes) {
    const auto renderable = std::dynamic_pointer_cast<Renderable>(node);
    if (renderable) {
      renderables.insert(renderable);
    }
    const auto camera = std::dynamic_pointer_cast<Camera>(node);
    if (camera) {
      cameras.insert(camera);
    }
  }
}

void RenderSystem::NotifyOfNodeDetachment(
    const std::shared_ptr<Node>& new_node) {
  const std::vector<std::shared_ptr<Node>> nodes =
      CollectPreOrderNodes(new_node);
  for (const std::shared_ptr<Node>& node : nodes) {
    const auto renderable = std::dynamic_pointer_cast<Renderable>(node);
    if (renderable) {
      renderables.erase(renderable);
    }
    const auto camera = std::dynamic_pointer_cast<Camera>(node);
    if (camera) {
      cameras.erase(camera);
    }
  }
}

void RenderSystem::LateUpdate(float delta_seconds) {
  std::vector<std::shared_ptr<Camera>> ordered_cameras(cameras.begin(),
                                                       cameras.end());

  std::sort(
      ordered_cameras.begin(), ordered_cameras.end(),
      [](const std::shared_ptr<Camera>& a, const std::shared_ptr<Camera>& b) {
        return a->sort_order - b->sort_order;
      });

  for (const std::shared_ptr<Camera>& camera : ordered_cameras) {
    bool clear_depth = bool(camera->clear_flags & (Camera::ClearFlags::Depth));
    bool clear_colour =
        bool(camera->clear_flags & (Camera::ClearFlags::Colour));
    glClear((GL_COLOR_BUFFER_BIT * clear_colour) |
            (GL_DEPTH_BUFFER_BIT * clear_depth));
    const glm::mat4 pv = camera->GetProjectionView(1280.f / 720.f);
    for (const std::shared_ptr<Renderable>& renderable : renderables) {
      // printf("Camera * obj render\n");
      renderable->Render(
          std::static_pointer_cast<RenderSystem>(this->shared_from_this()), pv);
    }
  }
}