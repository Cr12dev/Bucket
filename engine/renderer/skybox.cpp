#include "skybox.hpp"
#include "camera.hpp"

void skybox::init()
{
  shader_ = std::make_shared<shader>("shaders/sky.vert", "shaders/sky.frag");
  quad_ = mesh::fullscreen_quad();
}

void skybox::render(const camera& cam, const atmosphere& at) const
{
  if (!shader_ || !quad_.valid()) return;

  mat4 inv_view_proj = cam.view_projection().inverse();

  glDepthMask(GL_FALSE);

  shader_->bind();
  shader_->set_uniform("u_inv_view_proj", inv_view_proj.data());
  shader_->set_uniform("u_camera_pos", cam.position().x, cam.position().y, cam.position().z);
  shader_->set_uniform("u_sun_dir", sun_dir_.x, sun_dir_.y, sun_dir_.z);
  shader_->set_uniform("u_sky_top", at.sky_top.x, at.sky_top.y, at.sky_top.z);
  shader_->set_uniform("u_sky_horizon", at.sky_horizon.x, at.sky_horizon.y, at.sky_horizon.z);
  shader_->set_uniform("u_sky_ground", at.sky_ground.x, at.sky_ground.y, at.sky_ground.z);

  quad_.bind();
  glDrawElements(GL_TRIANGLES, quad_.index_count(), GL_UNSIGNED_INT, nullptr);

  glDepthMask(GL_TRUE);
}
