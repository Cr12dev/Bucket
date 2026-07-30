#pragma once

#include "mat4.hpp"
#include "vec3.hpp"

inline mat4 make_transform(const vec3& position, const vec3& rotation, const vec3& scale) {
  mat4 t = mat4::translate(position);
  t = t * mat4::rotate_x(rotation.x);
  t = t * mat4::rotate_y(rotation.y);
  t = t * mat4::rotate_z(rotation.z);
  t = t * mat4::scale(scale);
  return t;
}

inline mat4 make_transform(const vec3& position, const vec3& rotation) {
  return make_transform(position, rotation, vec3(1.0f));
}
