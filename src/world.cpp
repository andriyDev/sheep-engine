
#include "world.h"

#include <glog/logging.h>

#include "engine.h"

void World::SetRoot(const std::shared_ptr<Node>& new_root) {
  if (root) {
    PropagateNodeDetachment(root);

    root->world = std::shared_ptr<World>();
  }

  CHECK(new_root.get());
  root = new_root;
  root->world = this->shared_from_this();

  PropagateNodeAttachment(root);
}

const std::shared_ptr<System>& World::AddSystem(
    const std::shared_ptr<System>& new_system, int index) {
  // Only allow adding `new_system` if it is not a part of some world.
  CHECK(!new_system->GetWorld().get());
  const std::shared_ptr<World> this_ptr = this->shared_from_this();
  new_system->engine = GetEngine();
  new_system->world = this_ptr;
  if (index < 0) {
    index += systems.size() + 1;
    CHECK(index >= 0);
  } else {
    CHECK(index < systems.size());
  }
  systems.insert(systems.begin() + index, new_system);

  if (is_initialized) {
    new_system->Init();

    if (root) {
      new_system->NotifyOfNodeAttachment(root);
    }
    GetEngine()->PropagateSystemAddition(this_ptr, new_system);
  }
  return new_system;
}

void World::RemoveSystem(const std::shared_ptr<System>& system) {
  const auto it = std::find(systems.begin(), systems.end(), system);
  if (it != systems.end()) {
    if (is_initialized) {
      GetEngine()->PropagateSystemRemoval(this->shared_from_this(), system);
    }

    system->engine = std::shared_ptr<Engine>();
    system->world = std::shared_ptr<World>();
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
  if (!is_initialized) {
    return;
  }
  for (const std::shared_ptr<System>& system : systems) {
    system->NotifyOfNodeAttachment(node);
  }
}

void World::PropagateNodeDetachment(const std::shared_ptr<Node>& node) {
  if (!is_initialized) {
    return;
  }
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

    if (root) {
      system->NotifyOfNodeAttachment(root);
    }
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
