
#pragma once

#include <unordered_set>

#include "nodes/camera.h"
#include "nodes/node.h"
#include "systems/system.h"

class RenderSystem;

class Renderable {
 protected:
  virtual void Render(const std::shared_ptr<RenderSystem>& system,
                      const glm::mat4& ProjectionView) = 0;

  friend class RenderSystem;
};

class RenderSystem : public System {
 public:
  void NotifyOfNodeAttachment(const std::shared_ptr<Node>& new_node) override;
  void NotifyOfNodeDetachment(const std::shared_ptr<Node>& new_node) override;

  void LateUpdate(float delta_seconds) override;

 private:
  std::unordered_set<std::shared_ptr<Renderable>> renderables;
  std::unordered_set<std::shared_ptr<Camera>> cameras;
};
