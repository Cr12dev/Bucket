#pragma once

#include "math/vec3.hpp"

enum class light_type : int {
  directional = 0,
  point = 1,
  spot = 2,
};

struct light {
  light_type type = light_type::directional;
  vec3 color = vec3(1.0f, 1.0f, 1.0f);
  float intensity = 1.0f;
  vec3 position = vec3(0.0f, 0.0f, 0.0f);
  float range = 20.0f;
  vec3 direction = vec3(0.0f, -1.0f, 0.0f);
  float spot_cos_inner = 0.9f;
  float spot_cos_outer = 0.7f;
};
