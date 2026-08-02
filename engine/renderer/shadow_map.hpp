#pragma once

#include "shader.hpp"
#include "mesh.hpp"
#include "math/mat4.hpp"
#include "math/vec3.hpp"

#include <memory>

// Renders depth from the directional light's point of view into a texture,
// so the lighting shader can sample it for soft (PCF) shadows.
class shadow_map {
public:
  void init(int size = 2048);

  // Binds the depth FBO and prepares the light-space matrix, centered
  // around `focus` and lit from the given (world) light direction.
  void begin(const vec3& light_dir, const vec3& focus);
  void end();

  // Draws one box into the shadow map with the current light matrix.
  void draw(const mat4& model);

  void bind_depth(int unit) const;

  const mat4& light_view_proj() const { return light_view_proj_; }
  bool ready() const { return fbo_ != 0; }

private:
  unsigned int fbo_ = 0;
  unsigned int depth_tex_ = 0;
  int size_ = 2048;
  std::shared_ptr<shader> depth_shader_;
  mesh cube_;
  mat4 light_view_proj_ = mat4::identity();
};
