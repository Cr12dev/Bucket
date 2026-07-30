#pragma once

#include <glm/glm.hpp>
#include <cmath>

struct vec3 {
  float x, y, z;

  vec3() : x(0), y(0), z(0) {}
  vec3(float v) : x(v), y(v), z(v) {}
  vec3(float x, float y, float z) : x(x), y(y), z(z) {}

  vec3 operator+(const vec3& rhs) const { return { x + rhs.x, y + rhs.y, z + rhs.z }; }
  vec3 operator-(const vec3& rhs) const { return { x - rhs.x, y - rhs.y, z - rhs.z }; }
  vec3 operator*(float s) const { return { x * s, y * s, z * s }; }
  vec3 operator/(float s) const { return { x / s, y / s, z / s }; }

  vec3& operator+=(const vec3& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
  vec3& operator-=(const vec3& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
  vec3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }

  vec3 operator-() const { return { -x, -y, -z }; }

  float length() const { return std::sqrt(x * x + y * y + z * z); }
  vec3 normalized() const { float l = length(); return l > 0 ? *this / l : vec3(0); }

  static vec3 up()    { return { 0, 1, 0 }; }
  static vec3 down()  { return { 0, -1, 0 }; }
  static vec3 right() { return { 1, 0, 0 }; }
  static vec3 left()  { return { -1, 0, 0 }; }
  static vec3 forward(){ return { 0, 0, -1 }; }
  static vec3 back()  { return { 0, 0, 1 }; }
};

inline vec3 operator*(float s, const vec3& v) { return v * s; }

inline float dot(const vec3& a, const vec3& b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline vec3 cross(const vec3& a, const vec3& b) {
  return {
    a.y * b.z - a.z * b.y,
    a.z * b.x - a.x * b.z,
    a.x * b.y - a.y * b.x
  };
}

inline vec3 lerp(const vec3& a, const vec3& b, float t) {
  return a + (b - a) * t;
}
