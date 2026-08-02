#include "shadow_map.hpp"

#include "render_command.hpp"
#include <glad.h>
#include <glm/glm.hpp>
#include <cmath>
#include <cstdio>

void shadow_map::init(int size)
{
  size_ = size;
  cube_ = mesh::cube();

  depth_shader_ = std::make_shared<shader>(
    "shaders/shadow.vert", "shaders/shadow.frag"
  );

  glGenFramebuffers(1, &fbo_);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

  glGenTextures(1, &depth_tex_);
  glBindTexture(GL_TEXTURE_2D, depth_tex_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24,
               size_, size_, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  float border[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                         GL_TEXTURE_2D, depth_tex_, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::printf("[shadow_map] framebuffer incomplete!\n");
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void shadow_map::begin(const vec3& light_dir, const vec3& focus)
{
  vec3 dir = -light_dir.normalized();

  vec3 up = vec3::up();
  if (std::abs(dot(dir, up)) > 0.9f) {
    up = vec3::forward();
  }

  vec3 eye = focus - dir * 80.0f;
  mat4 view = mat4::look_at(eye, focus, up);
  mat4 proj = mat4(glm::ortho(-45.0f, 45.0f, -45.0f, 45.0f, 1.0f, 170.0f));
  light_view_proj_ = proj * view;

  glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
  glViewport(0, 0, size_, size_);
  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);
  glClear(GL_DEPTH_BUFFER_BIT);

  depth_shader_->bind();
  depth_shader_->set_uniform("u_light_view_proj", light_view_proj_.data());
}

void shadow_map::end()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void shadow_map::draw(const mat4& model)
{
  depth_shader_->set_uniform("u_model", model.data());
  render_command::draw_indexed(cube_);
}

void shadow_map::bind_depth(int unit) const
{
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(GL_TEXTURE_2D, depth_tex_);
}
