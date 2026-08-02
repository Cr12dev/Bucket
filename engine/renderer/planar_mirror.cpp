#include "planar_mirror.hpp"

#include <glad.h>
#include <cstdio>

void planar_mirror::init()
{
  glGenFramebuffers(1, &fbo_);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

  glGenTextures(1, &color_tex_);
  glBindTexture(GL_TEXTURE_2D, color_tex_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glGenRenderbuffers(1, &depth_rbo_);
  glBindRenderbuffer(GL_RENDERBUFFER, depth_rbo_);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D, color_tex_, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, depth_rbo_);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::printf("[planar_mirror] framebuffer incomplete!\n");
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void planar_mirror::resize(int w, int h)
{
  if (w == width_ && h == height_) return;

  width_ = w;
  height_ = h;

  glBindTexture(GL_TEXTURE_2D, color_tex_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

  glBindRenderbuffer(GL_RENDERBUFFER, depth_rbo_);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void planar_mirror::begin()
{
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
  glViewport(0, 0, width_, height_);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void planar_mirror::end()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void planar_mirror::bind_color(int unit) const
{
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(GL_TEXTURE_2D, color_tex_);
}
