#pragma once

#include <memory>
#include <string>
#include <cstdint>

#include "application.hpp"
#include "shader.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include "camera.hpp"
#include "core/ecs/scene.hpp"
#include "core/ecs/entity.hpp"
#include "skybox.hpp"

class editor {
public:
  editor();
  void run();

  void init_imgui();
  void shutdown_imgui();

private:
  enum class gizmo_mode { translate, rotate, scale };

  void awake();
  void start();
  void update(double dt);
  void render();

  void build_dockspace();
  void build_menu_bar();
  void build_viewport();
  void build_hierarchy();
  void build_properties();
  void build_status_bar();
  void build_map_dialogs();

  std::string entity_label(const entity& e) const;
  std::string make_unique_name(const std::string& base) const;
  void create_entity(const char* name);
  void duplicate_entity(const entity& e);
  void delete_entity(const entity& e);

  std::unique_ptr<application> app_;
  std::shared_ptr<shader> default_shader_;
  std::shared_ptr<shader> lighting_shader_;
  mesh cube_mesh_;
  texture white_tex_;
  camera camera_;
  scene scene_;
  entity selected_ = null_entity();
  skybox skybox_;

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
  bool dragging_ = false;
  double last_mx_ = 0.0;
  double last_my_ = 0.0;
  bool viewport_hovered_ = false;

  // window visibility
  bool show_viewport_ = true;
  bool show_hierarchy_ = true;
  bool show_properties_ = true;
  bool show_grid_ = true;
  bool show_demo_ = false;
  bool dockspace_init_ = false;

  // gizmo
  gizmo_mode gizmo_mode_ = gizmo_mode::translate;
  bool gizmo_snap_ = false;

  // map save/load
  bool map_popup_save_ = false;
  bool map_popup_load_ = false;
  char map_name_[64] = "level";
  std::string status_message_;
};
