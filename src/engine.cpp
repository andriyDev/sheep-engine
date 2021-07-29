
#include "engine.h"

#include <glog/logging.h>

std::shared_ptr<World> Engine::CreateWorld() {
  std::shared_ptr<World> world(new World());
  world->engine = this->shared_from_this();
  worlds.push_back(world);

  return world;
}

void Engine::RemoveWorld(const std::shared_ptr<World>& world) {
  const auto it = std::find(worlds.begin(), worlds.end(), world);
  if (it != worlds.end()) {
    if (world->is_initialized) {
      for (const std::shared_ptr<SuperSystem>& super_system : super_systems) {
        for (const std::shared_ptr<System>& system : world->GetSystems()) {
          super_system->NotifyOfSystemRemoval(world, system);
        }

        super_system->NotifyOfWorldDeletion(world);
      }
    }

    worlds.erase(it);
  }
}

void Engine::InitWorld(const std::shared_ptr<World>& world) {
  CHECK(is_initialized && world.get() && world->GetEngine().get() == this);

  if (world->is_initialized) {
    return;
  }

  world->Init();

  for (const std::shared_ptr<SuperSystem>& super_system : super_systems) {
    super_system->NotifyOfWorldInitialization(world);

    for (const std::shared_ptr<System>& system : world->GetSystems()) {
      super_system->NotifyOfSystemAddition(world, system);
    }
  }
}

const std::shared_ptr<SuperSystem>& Engine::AddSuperSystem(
    const std::shared_ptr<SuperSystem>& super_system, int index) {
  // Only allow adding `super_system` if it is not a part of some engine.
  CHECK(!super_system->GetEngine().get());
  super_system->engine = this->shared_from_this();
  if (index < 0) {
    index += super_systems.size() + 1;
    CHECK(index >= 0);
  } else {
    CHECK(index < super_systems.size());
  }
  super_systems.insert(super_systems.begin() + index, super_system);

  if (is_initialized) {
    super_system->Init();

    for (const std::shared_ptr<World>& world : worlds) {
      if (world->is_initialized) {
        super_system->NotifyOfWorldInitialization(world);
      }
    }
  }
  return super_system;
}

void Engine::RemoveSuperSystem(
    const std::shared_ptr<SuperSystem>& super_system) {
  const auto it =
      std::find(super_systems.begin(), super_systems.end(), super_system);
  if (it != super_systems.end()) {
    super_systems.erase(it);
  }
}

const std::vector<std::shared_ptr<World>>& Engine::GetWorlds() const {
  return worlds;
}
const std::vector<std::shared_ptr<SuperSystem>>& Engine::GetSuperSystems()
    const {
  return super_systems;
}

void Engine::Quit() { is_running = false; }

void Engine::Run(GLFWwindow* window) {
  Init();

  double previous_time = glfwGetTime();
  double game_time_offset = 0;

  while (is_running && !glfwWindowShouldClose(window)) {
    const double time = glfwGetTime();
    const double delta = time - previous_time;
    previous_time = time;

    // printf("Delta: %f\n", 1.0f / delta);

    Update(delta);

    const double fixed_update_delta = 1.0 / fixed_updates_per_second;
    game_time_offset += delta;
    unsigned int desired_frames =
        (unsigned int)(game_time_offset * fixed_updates_per_second);
    game_time_offset -= desired_frames * fixed_update_delta;

    // Ignore any frames passed `max_fixed_updates_per_frame`.
    desired_frames = std::min(desired_frames, max_fixed_updates_per_frame);
    for (int i = 0; i < desired_frames; i++) {
      FixedUpdate(fixed_update_delta);
    }

    LateUpdate(delta);

    glfwPollEvents();
  }
}

void Engine::Init() {
  if (is_initialized) {
    return;
  }
  is_initialized = true;
  is_running = true;
  for (const std::shared_ptr<SuperSystem>& super_system : super_systems) {
    super_system->Init();
  }
  for (const std::shared_ptr<World>& world : worlds) {
    InitWorld(world);
  }
}

void Engine::Update(float delta_seconds) {
  for (const std::shared_ptr<World>& world : worlds) {
    if (world->is_initialized) {
      world->Update(delta_seconds);
    }
  }
  for (const std::shared_ptr<SuperSystem>& super_system : super_systems) {
    super_system->Update(delta_seconds);
  }
}

void Engine::FixedUpdate(float delta_seconds) {
  for (const std::shared_ptr<World>& world : worlds) {
    if (world->is_initialized) {
      world->FixedUpdate(delta_seconds);
    }
  }
  for (const std::shared_ptr<SuperSystem>& super_system : super_systems) {
    super_system->FixedUpdate(delta_seconds);
  }
}

void Engine::LateUpdate(float delta_seconds) {
  for (const std::shared_ptr<World>& world : worlds) {
    if (world->is_initialized) {
      world->LateUpdate(delta_seconds);
    }
  }
  for (const std::shared_ptr<SuperSystem>& super_system : super_systems) {
    super_system->LateUpdate(delta_seconds);
  }
}

void Engine::PropagateSystemAddition(const std::shared_ptr<World>& world,
                                     const std::shared_ptr<System>& system) {
  for (const std::shared_ptr<SuperSystem>& super_system : super_systems) {
    super_system->NotifyOfSystemAddition(world, system);
  }
}

void Engine::PropagateSystemRemoval(const std::shared_ptr<World>& world,
                                    const std::shared_ptr<System>& system) {
  for (const std::shared_ptr<SuperSystem>& super_system : super_systems) {
    super_system->NotifyOfSystemRemoval(world, system);
  }
}
