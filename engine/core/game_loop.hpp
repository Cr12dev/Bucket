#pragma once

#include <memory>

#include "shader.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include "camera.hpp"
#include "application.hpp"
#include "core/ecs/scene.hpp"
#include "skybox.hpp"
#include "shadow_map.hpp"
#include "planar_mirror.hpp"

class buckit {
public:
  buckit();
  void run();

  // Loads a level file (<name>.lev) into the scene. Returns false if the
  // map could not be found. All saved entities are replicated into the
  // scene (with their ids) and can then be edited from game code.
  bool load_map(const char* path);

  // Simple accessor: edit a map object by its id. Each object keeps the
  // id it had when the map was saved. Returns nullptr if it doesn't exist.
  transform* object(uint32_t id);

private:
  void awake();
  void start();
  void update(double dt);
  void render();
  void select_shader(int index);

  // Builds the game scenario (CS 1.6-style level) from engine prefabs.
  void build_map();
  // Fires a raycast from the camera center (crosshair) and flashes the hit.
  void shoot_ray();
  // Draws a simple FPS crosshair over the scene.
  void draw_crosshair(int w, int h);
  // Uploads the entity's paint component (colors + albedo/normal/roughness/
  // emission and per-face maps) to the given shader. Applies the hit-flash tint.
  void bind_material(entity e, shader* s);

  std::unique_ptr<application> app_;
  std::shared_ptr<shader> default_shader_;
  std::shared_ptr<shader> basic_shader_;
  std::shared_ptr<shader> checker_shader_;
  std::shared_ptr<shader> pulse_shader_;
  std::shared_ptr<shader> lighting_shader_;
  shader* active_shader_ = nullptr;
  float elapsed_ = 0.0f;
  mesh cube_mesh_;
  texture checker_;
  camera camera_;
  scene scene_;
  skybox skybox_;
  shadow_map shadow_map_;
  planar_mirror mirror_;

  vec3 sun_dir_ = vec3(0.2f, 0.8f, 0.3f);
  entity player_entity_ = null_entity();
  camera mirror_camera_;

  // raycast feedback
  entity hit_flash_ = null_entity();
  float hit_flash_timer_ = 0.0f;
  float shoot_cooldown_ = 0.0f;

  fps_counter fps_;
};
