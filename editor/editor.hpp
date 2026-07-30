#pragma once

#include <memory>
#include <vector>
#include <cstdint>

#include "application.hpp"
#include "shader.hpp"
#include "mesh.hpp"
#include "camera.hpp"
#include "core/ecs/scene.hpp"
#include "core/ecs/entity.hpp"

class editor {
public:
  editor();
  void run();

  void init_imgui();
  void shutdown_imgui();

private:
  void awake();
  void start();
  void update(double dt);
  void render();

  void build_menu_bar();
  void build_viewport();
  void build_hierarchy();
  void build_properties();

  std::unique_ptr<application> app_;
  std::shared_ptr<shader> default_shader_;
  mesh cube_mesh_;
  camera camera_;
  scene scene_;
  entity selected_ = null_entity();

  // viewport framebuffer
  unsigned int fbo_ = 0;
  unsigned int color_tex_ = 0;
  unsigned int rbo_ = 0;
  int vp_w_ = 1280;
  int vp_h_ = 720;

  // editor camera
  float cam_yaw_ = 0.0f;
  float cam_pitch_ = -0.3f;
  float cam_dist_ = 5.0f;
  float cam_speed_ = 4.0f;
  bool dragging_ = false;
  double last_mx_ = 0.0;
  double last_my_ = 0.0;

  bool show_demo_ = false;
  bool show_grid_ = true;
};
