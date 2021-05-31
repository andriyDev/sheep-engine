
#include "world.h"

#include <assert.h>

#include "engine.h"

void World::SetRoot(const std::shared_ptr<Node>& new_root) {
  if (root) {
    PropagateNodeDetachment(root);
    root->world = std::shared_ptr<World>();
  }

  assert(new_root.get());
  root = new_root;
  root->world = this->shared_from_this();

  PropagateNodeAttachment(root);
}

const std::shared_ptr<System>& World::AddSystem(
    const std::shared_ptr<System>& new_system, int index) {
  // Only allow adding `new_system` if it is not a part of some world.
  assert(!new_system->GetWorld().get());
  const std::shared_ptr<World> this_ptr = this->shared_from_this();
  new_system->world = this_ptr;
  if (index < 0) {
    index += systems.size() + 1;
    assert(index >= 0);
  } else {
    assert(index < systems.size());
  }
  systems.insert(systems.begin() + index, new_system);

  if (is_initialized) {
    new_system->Init();
  }

  if (root) {
    new_system->NotifyOfNodeAttachment(root);
  }

  GetEngine()->PropagateSystemAddition(this_ptr, new_system);
  return new_system;
}

void World::RemoveSystem(const std::shared_ptr<System>& system) {
  const auto it = std::find(systems.begin(), systems.end(), system);
  if (it != systems.end()) {
    GetEngine()->PropagateSystemRemoval(this->shared_from_this(), system);

    systems.erase(it);
  }
}

std::shared_ptr<Node> World::CreateEmptyRoot() {
  std::shared_ptr<Node> root(new Node());
  SetRoot(root);
  return root;
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

void World::Init() {
  if (is_initialized) {
    return;
  }
  is_initialized = true;
  for (const std::shared_ptr<System>& system : systems) {
    system->Init();
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
