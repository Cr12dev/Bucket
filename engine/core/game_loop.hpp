#pragma once

#include <memory>

#include "shader.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include "camera.hpp"
#include "application.hpp"
#include "core/ecs/scene.hpp"

class buckit {
public:
  buckit();
  void run();

private:
  void awake();
  void start();
  void update(double dt);
  void render();

  std::unique_ptr<application> app_;
  std::shared_ptr<shader> default_shader_;
  mesh cube_mesh_;
  texture checker_;
  camera camera_;
  scene scene_;

  fps_counter fps_;
};
