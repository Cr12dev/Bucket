#pragma once

#include <cmath>

struct vec2 {
  float x, y;

  vec2() : x(0), y(0) {}
  vec2(float v) : x(v), y(v) {}
  vec2(float x, float y) : x(x), y(y) {}

  vec2 operator+(const vec2& rhs) const { return { x + rhs.x, y + rhs.y }; }
  vec2 operator-(const vec2& rhs) const { return { x - rhs.x, y - rhs.y }; }
  vec2 operator*(float s) const { return { x * s, y * s }; }
  vec2 operator/(float s) const { return { x / s, y / s }; }

  float length() const { return std::sqrt(x * x + y * y); }
};
