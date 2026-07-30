#pragma once

#include "core/ecs/component.hpp"
#include "core/math/vec3.hpp"
#include "core/math/mat4.hpp"

struct transform : public component {
  vec3 position = vec3(0.0f);
  vec3 rotation = vec3(0.0f);
  vec3 scale    = vec3(1.0f);

  mat4 matrix() const;
};
