#pragma once

#include "shader.hpp"
#include "mesh.hpp"
#include "math/vec3.hpp"
#include "core/atmosphere.hpp"
#include <memory>

class camera;

class skybox {
public:
  void init();
  void render(const camera& cam, const atmosphere& at = atmosphere()) const;

  const vec3& sun_dir() const { return sun_dir_; }
  void set_sun_dir(const vec3& d) { sun_dir_ = d; }

private:
  std::shared_ptr<shader> shader_;
  mesh quad_;
  vec3 sun_dir_ = { 0.2f, 0.8f, 0.3f };
};
