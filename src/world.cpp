
#include "world.h"

#include <assert.h>

World::World() {
  root = std::shared_ptr<Node>(new Node());
  root->world = this->shared_from_this();

  PropagateNodeAttachment(root);
}

void World::SetRoot(const std::shared_ptr<Node>& new_root) {
  PropagateNodeDetachment(root);
  root->world = std::shared_ptr<World>();

  assert(new_root.get());
  root = new_root;
  root->world = this->shared_from_this();

  PropagateNodeAttachment(root);
}

void World::AddSystem(const std::shared_ptr<System>& new_system, int index) {
  // Only allow adding `new_system` if it is not a part of some world.
  assert(!new_system->GetWorld().get());
  new_system->world = this->shared_from_this();
  if (index < 0) {
    index += systems.size() + 1;
    assert(index >= 0);
  } else {
    assert(index < systems.size());
  }
  systems.insert(systems.begin() + index, new_system);

  new_system->NotifyOfNodeAttachment(root);
}

void World::RemoveSystem(const std::shared_ptr<System>& system) {
  const auto it = std::find(systems.begin(), systems.end(), system);
  if (it != systems.end()) {
    systems.erase(it);
  }
}

std::shared_ptr<Node> World::GetRoot() const { return root; }

std::shared_ptr<Engine> World::GetEngine() const { return engine.lock(); }

const std::vector<std::shared_ptr<System>>& World::GetSystems() const {
  return systems;
}

void World::PropagateNodeAttachment(const std::shared_ptr<Node>& node) {
  for (const std::shared_ptr<System>& system : systems) {
    system->NotifyOfNodeAttachment(node);
  }
}

void World::PropagateNodeDetachment(const std::shared_ptr<Node>& node) {
  for (const std::shared_ptr<System>& system : systems) {
    system->NotifyOfNodeDetachment(node);
  }
}

void World::Update(float delta_seconds) {
  for (const std::shared_ptr<System>& system : systems) {
    system->Update(delta_seconds);
  }
}

void World::FixedUpdate(float delta_seconds) {
  for (const std::shared_ptr<System>& system : systems) {
    system->FixedUpdate(delta_seconds);
  }
}

void World::LateUpdate(float delta_seconds) {
  for (const std::shared_ptr<System>& system : systems) {
    system->LateUpdate(delta_seconds);
  }
}
