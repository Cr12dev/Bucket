#pragma once

#include <glm/glm.hpp>

struct vec4 {
  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
  float w = 1.0f;

  vec4() = default;
  vec4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
  vec4(float s) : x(s), y(s), z(s), w(s) {}
  vec4(const glm::vec4& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
};
