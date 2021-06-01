
#include "systems/input_system.h"

#include <stdio.h>

#define GLFW_KEY_SIZE (GLFW_KEY_LAST + 1)

InputSuperSystem::InputSuperSystem(GLFWwindow* window_) : window(window_) {}

void InputSuperSystem::SetMouseLock(bool lock) {
  glfwSetInputMode(window, GLFW_CURSOR,
                   lock ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

InputSuperSystem::ButtonDefinition InputSuperSystem::ButtonDefinition::Key(
    unsigned int key, int modifiers) {
  ButtonDefinition definition;
  definition.type = Type::Key;
  definition.key = key;
  definition.modifiers = modifiers;
  return definition;
}

InputSuperSystem::ButtonDefinition
InputSuperSystem::ButtonDefinition::MouseButton(unsigned int mouse_button,
                                                int modifiers) {
  ButtonDefinition definition;
  definition.type = Type::MouseButton;
  definition.key = mouse_button + GLFW_KEY_SIZE;
  definition.modifiers = modifiers;
  return definition;
}

int CountModBits(int value) {
  return bool(value & GLFW_MOD_SHIFT) + bool(value & GLFW_MOD_CONTROL) +
         bool(value & GLFW_MOD_ALT) + bool(value & GLFW_MOD_SUPER);
}

void InputSuperSystem::CreateButton(
    const std::string& name, const std::vector<ButtonDefinition>& definitions) {
  // Try to create the button.
  auto button_pair = buttons.insert(std::make_pair(name, Button{}));
  // If the button already exists, clear it and insert a new one.
  if (!button_pair.second) {
    ClearButton(name);
    button_pair = buttons.insert(std::make_pair(name, Button{}));
  }
  // Get a reference to the new button.
  Button& button = button_pair.first->second;
  // Go through each definition.
  for (const ButtonDefinition& definition : definitions) {
    // Compute the key id (depending on if it's a MouseButton or Key).
    unsigned int key =
        definition.key -
        int(definition.type == ButtonDefinition::Type::MouseButton) *
            GLFW_KEY_SIZE;
    // Add the key + modifier to the button.
    button.keys_with_modifier.push_back(
        std::make_pair(key, definition.modifiers));
    // Increment the watch use for this key + modifier.
    IncrementWatchUse(key, definition.modifiers);
  }
}

void InputSuperSystem::ClearButton(const std::string& name) {
  // Find the button.
  auto button_it = buttons.find(name);
  if (button_it == buttons.end()) {
    fprintf(stderr, "Button \"%s\" does not exist.\n", name.c_str());
    return;
  }
  // Go through all button keys and erase them.
  for (const auto& [button, modifiers] : button_it->second.keys_with_modifier) {
    DecrementWatchUse(button, modifiers);
  }
  // Erase the button.
  buttons.erase(button_it);
}

std::vector<
    std::pair<std::string, std::vector<InputSuperSystem::ButtonDefinition>>>
InputSuperSystem::GetButtons() const {
  std::vector<
      std::pair<std::string, std::vector<InputSuperSystem::ButtonDefinition>>>
      result;
  result.reserve(buttons.size());
  // Go through each button.
  for (const auto& [name, button] : buttons) {
    std::vector<InputSuperSystem::ButtonDefinition> definitions;
    definitions.reserve(button.keys_with_modifier.size());
    // Go through each key for that button.
    for (const auto& [button, modifiers] : button.keys_with_modifier) {
      // Create a new definition, labeling it correctly if it is a MouseButton
      // or Key.
      ButtonDefinition definition;
      definition.type = button >= GLFW_KEY_SIZE
                            ? ButtonDefinition::Type::MouseButton
                            : ButtonDefinition::Type::Key;
      definition.key =
          button >= GLFW_KEY_SIZE ? button - GLFW_KEY_SIZE : button;
      definition.modifiers = modifiers;
      definitions.push_back(definition);
    }
    result.push_back(std::make_pair(name, definitions));
  }
  return result;
}

InputSuperSystem::AxisDefinition InputSuperSystem::AxisDefinition::Key(
    unsigned int key, float weight) {
  AxisDefinition definition;
  definition.type = Type::Key;
  definition.key = key;
  definition.weight = weight;
  return definition;
}

InputSuperSystem::AxisDefinition InputSuperSystem::AxisDefinition::MouseButton(
    unsigned int mouse_button, float weight) {
  AxisDefinition definition;
  definition.type = Type::MouseButton;
  definition.mouse_button = mouse_button + GLFW_KEY_SIZE;
  definition.weight = weight;
  return definition;
}

InputSuperSystem::AxisDefinition InputSuperSystem::AxisDefinition::MouseMove(
    Direction direction, float weight) {
  AxisDefinition definition;
  definition.type = Type::MouseMove;
  definition.mouse_move = direction;
  definition.weight = weight;
  return definition;
}

InputSuperSystem::AxisDefinition InputSuperSystem::AxisDefinition::Scroll(
    Direction direction, float weight) {
  AxisDefinition definition;
  definition.type = Type::Scroll;
  definition.scroll = direction;
  definition.weight = weight;
  return definition;
}

void InputSuperSystem::CreateAxis(
    const std::string& name, const std::vector<AxisDefinition>& definitions) {
  // Try to create the axis.
  auto axis_pair = axes.insert(std::make_pair(name, Axis()));
  if (!axis_pair.second) {
    ClearAxis(name);
    axis_pair = axes.insert(std::make_pair(name, Axis()));
  }
  Axis& axis = axis_pair.first->second;
  for (const AxisDefinition& definition : definitions) {
    switch (definition.type) {
      case AxisDefinition::Type::Key:
      case AxisDefinition::Type::MouseButton: {
        unsigned int key_id =
            definition.key -
            int(definition.type == AxisDefinition::Type::MouseButton) *
                GLFW_KEY_SIZE;
        axis.key_weights.push_back(std::make_pair(key_id, definition.weight));
        IncrementWatchUse(key_id, 0);
        break;
      }
      case AxisDefinition::Type::MouseMove:
        switch (definition.scroll) {
          case AxisDefinition::Direction::Horizontal:
            axis.mouse_move_weights.x = definition.weight;
            break;
          case AxisDefinition::Direction::Vertical:
            axis.mouse_move_weights.y = definition.weight;
            break;
          default:
            throw "Non-exhaustive switch!";
        }
        break;
      case AxisDefinition::Type::Scroll:
        switch (definition.scroll) {
          case AxisDefinition::Direction::Horizontal:
            axis.scroll_weights.x = definition.weight;
            break;
          case AxisDefinition::Direction::Vertical:
            axis.scroll_weights.y = definition.weight;
            break;
          default:
            throw "Non-exhaustive switch!";
        }
        break;
      default:
        throw "Non-exhaustive switch!";
    }
  }
}

void InputSuperSystem::ClearAxis(const std::string& name) {
  auto axis_it = axes.find(name);
  if (axis_it == axes.end()) {
    fprintf(stderr, "Axis \"%s\" does not exist.\n", name.c_str());
    return;
  }
  Axis& axis = axis_it->second;
  for (const auto& [key, _] : axis.key_weights) {
    DecrementWatchUse(key, 0);
  }
  axes.erase(axis_it);
}

std::vector<
    std::pair<std::string, std::vector<InputSuperSystem::AxisDefinition>>>
InputSuperSystem::GetAxes() const {
  std::vector<
      std::pair<std::string, std::vector<InputSuperSystem::AxisDefinition>>>
      result;
  result.reserve(axes.size());
  for (const auto& [name, axis] : axes) {
    result.push_back(
        std::make_pair(name, std::vector<InputSuperSystem::AxisDefinition>()));
    std::vector<InputSuperSystem::AxisDefinition>& definitions =
        result.back().second;
    if (axis.mouse_move_weights[0] != 0) {
      definitions.push_back(AxisDefinition::MouseMove(
          AxisDefinition::Direction::Horizontal, axis.mouse_move_weights[0]));
    }
    if (axis.mouse_move_weights[1] != 0) {
      definitions.push_back(AxisDefinition::MouseMove(
          AxisDefinition::Direction::Vertical, axis.mouse_move_weights[1]));
    }
    if (axis.scroll_weights[0] != 0) {
      definitions.push_back(AxisDefinition::Scroll(
          AxisDefinition::Direction::Horizontal, axis.scroll_weights[0]));
    }
    if (axis.scroll_weights[1] != 0) {
      definitions.push_back(AxisDefinition::Scroll(
          AxisDefinition::Direction::Vertical, axis.scroll_weights[1]));
    }
    definitions.reserve(definitions.size() + axis.key_weights.size());
    for (const auto& [key, weight] : axis.key_weights) {
      AxisDefinition definition;
      definition.type = key >= GLFW_KEY_SIZE ? AxisDefinition::Type::MouseButton
                                             : AxisDefinition::Type::Key;
      definition.key = key >= GLFW_KEY_SIZE ? key - GLFW_KEY_SIZE : key;
      definition.weight = weight;
      definitions.push_back(definition);
    }
  }
  return result;
}

bool InputSuperSystem::IsButtonPressed(const std::string& name) const {
  const auto& button_it = buttons.find(name);
  if (button_it == buttons.end()) {
    fprintf(stderr, "Invalid button name \"%s\"\n", name.c_str());
    return false;
  }
  for (const auto& [key, modifiers] : button_it->second.keys_with_modifier) {
    const KeyWatch* watch = GetWatch(key, modifiers);
    if (!watch) {
      continue;
    }
    if (watch->is_pressed) {
      return true;
    }
  }
  return false;
}

bool InputSuperSystem::IsButtonDown(const std::string& name) const {
  const auto& button_it = buttons.find(name);
  if (button_it == buttons.end()) {
    fprintf(stderr, "Invalid button name \"%s\"\n", name.c_str());
    return false;
  }
  for (const auto& [key, modifiers] : button_it->second.keys_with_modifier) {
    const KeyWatch* watch = GetWatch(key, modifiers);
    if (!watch) {
      continue;
    }
    if (watch->is_down) {
      return true;
    }
  }
  return false;
}

bool InputSuperSystem::IsButtonReleased(const std::string& name) const {
  const auto& button_it = buttons.find(name);
  if (button_it == buttons.end()) {
    fprintf(stderr, "Invalid button name \"%s\"\n", name.c_str());
    return false;
  }
  for (const auto& [key, modifiers] : button_it->second.keys_with_modifier) {
    const KeyWatch* watch = GetWatch(key, modifiers);
    if (!watch) {
      continue;
    }
    if (watch->is_released) {
      return true;
    }
  }
  return false;
}

float InputSuperSystem::GetAxisValue(const std::string& name) const {
  const auto& axis_it = axes.find(name);
  if (axis_it == axes.end()) {
    fprintf(stderr, "Invalid axis name \"%s\"\n", name.c_str());
    return 0;
  }
  const Axis& axis = axis_it->second;

  float value = 0;
  for (const auto& [key, weight] : axis.key_weights) {
    const KeyWatch* watch = GetWatch(key, 0);
    if (watch && watch->is_down) {
      value += weight;
    }
  }
  value += mouse_move[0] * axis.mouse_move_weights[0] +
           mouse_move[1] * axis.mouse_move_weights[1] +
           scroll[0] * axis.scroll_weights[0] +
           scroll[1] * axis.scroll_weights[1];
  return value;
}

glm::vec2 InputSuperSystem::GetMousePosition() const { return mouse_position; }

bool InputSuperSystem::IsMouseInWindow() const { return is_mouse_in_window; }

void InputSuperSystem::Init() {
  glfwSetWindowUserPointer(window, this);
  glfwSetKeyCallback(window, InputSuperSystem::KeyCallback);
  glfwSetMouseButtonCallback(window, InputSuperSystem::MouseButtonCallback);
  glfwSetCursorPosCallback(window, InputSuperSystem::MouseMoveCallback);
  glfwSetCursorEnterCallback(window, InputSuperSystem::MouseEnterCallback);
  glfwSetScrollCallback(window, InputSuperSystem::ScrollCallback);
}

void InputSuperSystem::LateUpdate(float delta_seconds) {
  mouse_move[0] = 0;
  mouse_move[1] = 0;
  scroll[0] = 0;
  scroll[1] = 0;

  for (auto& [key, watches] : key_watches) {
    for (auto& watch : watches) {
      watch.is_pressed = false;
      watch.is_released = false;
    }
  }
}

InputSuperSystem::KeyWatch* InputSuperSystem::GetWatch(unsigned int key,
                                                       int modifiers) {
  // Use the const version by casting to and from const.
  return const_cast<KeyWatch*>(
      const_cast<const InputSuperSystem*>(this)->GetWatch(key, modifiers));
}

const InputSuperSystem::KeyWatch* InputSuperSystem::GetWatch(
    unsigned int key, int modifiers) const {
  const auto& watches_it = this->key_watches.find(key);
  if (watches_it == this->key_watches.end()) {
    return nullptr;
  }
  for (const KeyWatch& watch : watches_it->second) {
    if (watch.modifiers == modifiers || watch.modifiers == 0) {
      return &watch;
    }
  }
  return nullptr;
}

void InputSuperSystem::IncrementWatchUse(unsigned int key, int modifiers) {
  // Ensure at least one watch exists for this key.
  auto watch_pair = key_watches.insert(std::make_pair(
      key, std::vector<KeyWatch>{KeyWatch{modifiers, 1, false, false, false}}));
  // If the watch was not inserted, either an existing watch must get an
  // additional use, or a new watch must be appended.
  if (!watch_pair.second) {
    // Count the bits of the modifiers (so watches can be sorted by the number
    // of modifiers).
    int mod_count = CountModBits(modifiers);
    // Get the watches for this button id.
    std::vector<KeyWatch>& watches = watch_pair.first->second;
    int i;
    bool insert_at_index = true;
    for (i = 0; i < watches.size(); i++) {
      KeyWatch& watch = watches[i];
      // If the modifiers match, just increment the uses and mark that we
      // don't need to insert.
      if (watch.modifiers == modifiers) {
        watch.uses++;
        insert_at_index = false;
        break;
      } else if (CountModBits(watch.modifiers) < mod_count) {
        // Otherwise, if this is the first watch with fewer mod bits that
        // present, break out.
        break;
      }
    }
    // If either we fell through, or found the first slot with fewer mod bits,
    // insert the new watch at that index.
    if (insert_at_index) {
      watches.insert(watches.begin() + i, {modifiers, 1, false, false, false});
    }
  }
}

void InputSuperSystem::DecrementWatchUse(unsigned int key, int modifiers) {
  auto watch_it = key_watches.find(key);
  if (watch_it == key_watches.end()) {
    throw "No such button entry.";
  }
  std::vector<KeyWatch>& watches = watch_it->second;
  for (int i = 0; i < watches.size(); i++) {
    if (watches[i].modifiers != modifiers) {
      continue;
    }
    watches[i].uses--;
    if (watches[i].uses == 0) {
      watches.erase(watches.begin() + i);
    }
  }
  throw "No such button watch.";
}

void InputSuperSystem::KeyCallback(GLFWwindow* window, int key, int scancode,
                                   int action, int mods) {
  InputSuperSystem* input =
      static_cast<InputSuperSystem*>(glfwGetWindowUserPointer(window));
  KeyWatch* watch = input->GetWatch(key, mods);
  if (!watch) {
    printf("No watch found: %d/%d\n", key, mods);
    return;
  }
  printf("Found watch: %d/%d\n", key, mods);
  if (action == GLFW_PRESS) {
    watch->is_pressed = true;
    watch->is_down = true;
  } else {
    watch->is_down = false;
    watch->is_released = true;
  }
}

void InputSuperSystem::MouseButtonCallback(GLFWwindow* window, int button,
                                           int action, int mods) {
  InputSuperSystem* input =
      static_cast<InputSuperSystem*>(glfwGetWindowUserPointer(window));
  KeyWatch* watch = input->GetWatch(button + GLFW_KEY_SIZE, mods);
  if (!watch) {
    return;
  }
  if (action == GLFW_PRESS) {
    watch->is_pressed = true;
    watch->is_down = true;
  } else {
    watch->is_down = false;
    watch->is_released = true;
  }
}

void InputSuperSystem::MouseMoveCallback(GLFWwindow* window, double x,
                                         double y) {
  InputSuperSystem* input =
      static_cast<InputSuperSystem*>(glfwGetWindowUserPointer(window));

  input->mouse_move += glm::vec2(x, y) - input->mouse_position;
  input->mouse_position = glm::vec2(x, y);
}

void InputSuperSystem::MouseEnterCallback(GLFWwindow* window, int entered) {
  InputSuperSystem* input =
      static_cast<InputSuperSystem*>(glfwGetWindowUserPointer(window));

  if (entered) {
    input->is_mouse_in_window = true;
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    input->mouse_position = glm::vec2(x, y);
  } else {
    input->is_mouse_in_window = false;
  }
}

void InputSuperSystem::ScrollCallback(GLFWwindow* window, double x, double y) {
  InputSuperSystem* input =
      static_cast<InputSuperSystem*>(glfwGetWindowUserPointer(window));

  input->scroll += glm::vec2(x, y);
}
