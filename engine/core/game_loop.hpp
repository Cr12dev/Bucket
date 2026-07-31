#pragma once

#include <memory>

#include "shader.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include "camera.hpp"
#include "application.hpp"
#include "core/ecs/scene.hpp"
#include "skybox.hpp"

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

  fps_counter fps_;
};
