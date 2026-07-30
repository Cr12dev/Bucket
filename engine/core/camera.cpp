#include "camera.hpp"
#include <cmath>

camera::camera()
{
  set_perspective(fov_, aspect_, near_, far_);
}

void camera::set_perspective(float fov_deg, float aspect, float near_plane, float far_plane)
{
  fov_ = fov_deg;
  aspect_ = aspect;
  near_ = near_plane;
  far_ = far_plane;
  projection_ = mat4::perspective(fov_deg * 3.14159265f / 180.0f, aspect, near_plane, far_plane);
}

void camera::set_position(const vec3& pos)
{
  position_ = pos;
  view_dirty_ = true;
}

void camera::set_yaw(float yaw_deg)
{
  yaw_ = yaw_deg;
  view_dirty_ = true;
}

void camera::set_pitch(float pitch_deg)
{
  pitch_ = pitch_deg;
  view_dirty_ = true;
}

void camera::move(const vec3& delta)
{
  position_ += delta;
  view_dirty_ = true;
}

void camera::rotate(float delta_yaw_deg, float delta_pitch_deg)
{
  yaw_ += delta_yaw_deg;
  pitch_ += delta_pitch_deg;

  if (pitch_ > 89.0f) pitch_ = 89.0f;
  if (pitch_ < -89.0f) pitch_ = -89.0f;

  view_dirty_ = true;
}

const mat4& camera::view_matrix() const
{
  if (view_dirty_) rebuild_view();
  return view_;
}

mat4 camera::view_projection() const
{
  return projection_ * view_matrix();
}

vec3 camera::forward() const
{
  float pitch_rad = pitch_ * 3.14159265f / 180.0f;
  float yaw_rad = yaw_ * 3.14159265f / 180.0f;
  return {
    std::cos(yaw_rad) * std::cos(pitch_rad),
    std::sin(pitch_rad),
    std::sin(yaw_rad) * std::cos(pitch_rad)
  };
}

vec3 camera::right() const
{
  vec3 fwd = forward();
  vec3 world_up = vec3::up();
  return cross(fwd, world_up).normalized();
}

vec3 camera::up() const
{
  return cross(right(), forward());
}

void camera::rebuild_view() const
{
  vec3 fwd = forward();
  vec3 target = position_ + fwd;
  view_ = mat4::look_at(position_, target, vec3::up());
  view_dirty_ = false;
}
