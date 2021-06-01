
#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <unordered_map>

#include "systems/super_system.h"

class InputSuperSystem : public SuperSystem {
 public:
  InputSuperSystem(GLFWwindow* window_);

  struct ButtonDefinition {
    enum class Type { Key, MouseButton } type;
    union {
      unsigned int key;
      unsigned int mouse_button;
    };
    int modifiers;

    static ButtonDefinition Key(unsigned int key, int modifiers);
    static ButtonDefinition MouseButton(unsigned int mouse_button,
                                        int modifiers);
  };

  // Sets whether the mouse is locked or not by `lock`.
  void SetMouseLock(bool lock);
  // Gets whether the mouse is currently locked.
  bool IsMouseLocked() const;

  // Creates (or updates) a button with `name` using `definitions`. If `name`
  // already exists, `definitions` will replace the existing definitions.
  void CreateButton(const std::string& name,
                    const std::vector<ButtonDefinition>& definitions);
  // Removes the button with `name`.
  void ClearButton(const std::string& name);
  // Gets all button names and the buttons they refer to.
  std::vector<std::pair<std::string, std::vector<ButtonDefinition>>>
  GetButtons() const;

  struct AxisDefinition {
    enum class Type { Key, MouseButton, MouseMove, Scroll } type;
    enum class Direction { Vertical, Horizontal };
    union {
      unsigned int key;
      unsigned int mouse_button;
      Direction mouse_move;
      Direction scroll;
    };
    float weight;

    static AxisDefinition Key(unsigned int key, float weight);
    static AxisDefinition MouseButton(unsigned int mouse_button, float weight);
    static AxisDefinition MouseMove(Direction direction, float weight);
    static AxisDefinition Scroll(Direction direction, float weight);
  };

  // Creates (or updates) an axis with `name` using `definitions`. If `name`
  // already exists, `definitions` will replace the existing definitions.
  void CreateAxis(const std::string& name,
                  const std::vector<AxisDefinition>& definitions);
  // Removes the axis with `name`.
  void ClearAxis(const std::string& name);
  // Gets all axis names and the definitions they refer to.
  std::vector<std::pair<std::string, std::vector<AxisDefinition>>> GetAxes()
      const;

  bool IsButtonPressed(const std::string& name) const;
  bool IsButtonDown(const std::string& name) const;
  bool IsButtonReleased(const std::string& name) const;

  float GetAxisValue(const std::string& name) const;

  glm::vec2 GetMousePosition() const;
  bool IsMouseInWindow() const;

 protected:
  void Init() override;

  void LateUpdate(float delta_seconds) override;

 private:
  GLFWwindow* window;

  struct KeyWatch {
    // The modifiers this watch looks for.
    int modifiers;
    // The number of references to this watch.
    unsigned int uses;

    // Whether the key has been pressed this frame.
    bool is_pressed;
    // Whether the key is currently pressed.
    bool is_down;
    // Whether the key has been released this frame.
    bool is_released;
  };

  const KeyWatch* GetWatch(unsigned int key, int modifiers) const;
  KeyWatch* GetWatch(unsigned int key, int modifiers);
  void IncrementWatchUse(unsigned int key, int modifiers);
  void DecrementWatchUse(unsigned int key, int modifiers);

  glm::vec2 mouse_move = glm::vec2(0, 0);
  glm::vec2 scroll = glm::vec2(0, 0);

  glm::vec2 mouse_position = {0, 0};
  bool is_mouse_in_window = false;

  struct Button {
    std::vector<std::pair<unsigned int, int>> keys_with_modifier;
  };
  std::unordered_map<std::string, Button> buttons;

  struct Axis {
    std::vector<std::pair<unsigned int, float>> key_weights;
    glm::vec2 mouse_move_weights = glm::vec2(0, 0);
    glm::vec2 scroll_weights = glm::vec2(0, 0);
  };
  std::unordered_map<std::string, Axis> axes;

  std::unordered_map<unsigned int, std::vector<KeyWatch>> key_watches;

  static void KeyCallback(GLFWwindow* window, int key, int scancode, int action,
                          int mods);
  static void MouseButtonCallback(GLFWwindow* window, int key, int action,
                                  int mods);
  static void MouseMoveCallback(GLFWwindow* window, double x, double y);
  static void MouseEnterCallback(GLFWwindow* window, int entered);
  static void ScrollCallback(GLFWwindow* window, double x, double y);
};
