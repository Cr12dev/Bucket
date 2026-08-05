#pragma once

#include <memory>

#include "shader.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include "camera.hpp"
#include "application.hpp"
#include "core/ecs/scene.hpp"
#include "core/atmosphere.hpp"
#include "skybox.hpp"
#include "shadow_map.hpp"
#include "planar_mirror.hpp"

class buckit {
public:
  buckit();
  void run();

  // Global atmosphere (ambient, fog, sky colors, sun). Edit it from
  // game code:  atmosphere_.ambient = vec3(...);  etc.
  atmosphere atmosphere_;

  // Loads a level file (<name>.lev) into the scene. Returns false if the
  // map could not be found. All saved entities are replicated into the
  // scene (with their ids) and can then be edited from game code.
  bool load_map(const char* path);

  // Simple accessor: edit a map object's transform by its id. Each object
  // keeps the id it had when the map was saved. Returns nullptr if missing.
  transform* object(uint32_t id);

  // Edit any component of a map object from game code, e.g.:
  //   component<paint>(id)->color = vec4(1, 0, 0, 1);
  //   component<transform>(id)->scale = vec3(2, 2, 2);
  // Returns nullptr if the object or component doesn't exist.
  template<typename T>
  T* component(uint32_t id) {
    return scene_.get_component<T>(entity{ id, 0 });
  }

  // Finds a map object by its mandatory id string (the one given to the
  // prefab). Returns null_entity() if no object has that id.
  entity find_object(const std::string& id) const;

  // Edit any component of a map object by its id string, e.g.:
  //   component<paint>("lampara")->emission_color = vec3(3, 2, 1);
  //   component<transform>("muro_norte")->position.y = 5;
  // Returns nullptr if the object or component doesn't exist.
  template<typename T>
  T* component(const std::string& id) {
    entity e = find_object(id);
    return e ? scene_.get_component<T>(e) : nullptr;
  }

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

  // Render pipeline passes (configured in start()).
  void render_mirror_pass();
  void render_shadow_pass();
  void render_main_pass();
  // Draws all scene entities (per-entity mesh + material shader, frustum culled).
  void draw_scene_entities();
  // Uploads the shared per-frame uniforms (view, fog, shadow, mirror,
  // lights) to the given shader. Used when an entity renders with its
  // own material shader instead of the global active one.
  void bind_scene_uniforms(shader* s);

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
  entity lamp_e_ = null_entity();  // demo object edited from update()
  int atmosphere_preset_ = 0;      // demo: [7] cycles atmosphere presets
  camera mirror_camera_;
  render_pipeline pipeline_;
  lighting lights_;
  int last_w_ = 0;
  int last_h_ = 0;

  // raycast feedback
  entity hit_flash_ = null_entity();
  float hit_flash_timer_ = 0.0f;
  float shoot_cooldown_ = 0.0f;

  fps_counter fps_;
};
