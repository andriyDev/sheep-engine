
#pragma once

#include <glm/glm.hpp>

#include "utility/hton.h"

template <>
inline glm::vec2 htob(glm::vec2 value) {
  value.x = htob(value.x);
  value.y = htob(value.y);
  return value;
}

template <>
inline glm::vec2 htol(glm::vec2 value) {
  value.x = htol(value.x);
  value.y = htol(value.y);
  return value;
}

template <>
inline glm::vec3 htob(glm::vec3 value) {
  value.x = htob(value.x);
  value.y = htob(value.y);
  value.z = htob(value.z);
  return value;
}

template <>
inline glm::vec3 htol(glm::vec3 value) {
  value.x = htol(value.x);
  value.y = htol(value.y);
  value.z = htol(value.z);
  return value;
}

template <>
inline glm::vec4 htob(glm::vec4 value) {
  value.x = htob(value.x);
  value.y = htob(value.y);
  value.z = htob(value.z);
  value.w = htob(value.w);
  return value;
}

template <>
inline glm::vec4 htol(glm::vec4 value) {
  value.x = htol(value.x);
  value.y = htol(value.y);
  value.z = htol(value.z);
  value.w = htol(value.w);
  return value;
}
