#pragma once

#include "math/vec3.hpp"
#include "math/mat4.hpp"

class camera {
public:
  camera();

  void set_perspective(float fov_deg, float aspect, float near_plane, float far_plane);

  void set_position(const vec3& pos);
  void set_yaw(float yaw_deg);
  void set_pitch(float pitch_deg);

  void move(const vec3& delta);
  void rotate(float delta_yaw_deg, float delta_pitch_deg);

  const vec3& position() const { return position_; }
  float yaw() const { return yaw_; }
  float pitch() const { return pitch_; }

  const mat4& view_matrix() const;
  const mat4& projection_matrix() const { return projection_; }
  mat4 view_projection() const;

  vec3 forward() const;
  vec3 right() const;
  vec3 up() const;

  float fov() const { return fov_; }
  float aspect() const { return aspect_; }

private:
  void rebuild_view() const;

  vec3 position_ = vec3(0.0f, 0.0f, 3.0f);
  float yaw_ = -90.0f;
  float pitch_ = 0.0f;
  float fov_ = 60.0f;
  float aspect_ = 16.0f / 9.0f;
  float near_ = 0.1f;
  float far_ = 100.0f;

  mutable mat4 view_;
  mutable mat4 projection_;
  mutable bool view_dirty_ = true;
};
