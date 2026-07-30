#pragma once

#include "vec3.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct mat4 {
  glm::mat4 m;

  mat4() : m(1.0f) {}
  explicit mat4(const glm::mat4& mat) : m(mat) {}

  const float* data() const { return glm::value_ptr(m); }
  float* data() { return glm::value_ptr(m); }

  mat4 operator*(const mat4& rhs) const { return mat4(m * rhs.m); }

  static mat4 identity() { return mat4(glm::mat4(1.0f)); }

  static mat4 translate(const vec3& t) {
    return mat4(glm::translate(glm::mat4(1.0f), glm::vec3(t.x, t.y, t.z)));
  }

  static mat4 rotate_x(float angle_rad) {
    return mat4(glm::rotate(glm::mat4(1.0f), angle_rad, glm::vec3(1, 0, 0)));
  }

  static mat4 rotate_y(float angle_rad) {
    return mat4(glm::rotate(glm::mat4(1.0f), angle_rad, glm::vec3(0, 1, 0)));
  }

  static mat4 rotate_z(float angle_rad) {
    return mat4(glm::rotate(glm::mat4(1.0f), angle_rad, glm::vec3(0, 0, 1)));
  }

  static mat4 scale(const vec3& s) {
    return mat4(glm::scale(glm::mat4(1.0f), glm::vec3(s.x, s.y, s.z)));
  }

  static mat4 perspective(float fov_rad, float aspect, float near, float far) {
    return mat4(glm::perspective(fov_rad, aspect, near, far));
  }

  static mat4 look_at(const vec3& eye, const vec3& center, const vec3& up) {
    return mat4(glm::lookAt(
      glm::vec3(eye.x, eye.y, eye.z),
      glm::vec3(center.x, center.y, center.z),
      glm::vec3(up.x, up.y, up.z)
    ));
  }

  vec3 transform_point(const vec3& p) const {
    glm::vec4 r = m * glm::vec4(p.x, p.y, p.z, 1.0f);
    return { r.x, r.y, r.z };
  }

  vec3 transform_vector(const vec3& v) const {
    glm::vec4 r = m * glm::vec4(v.x, v.y, v.z, 0.0f);
    return { r.x, r.y, r.z };
  }

  mat4 inverse() const {
    return mat4(glm::inverse(m));
  }

  mat4 transpose() const {
    return mat4(glm::transpose(m));
  }
};
