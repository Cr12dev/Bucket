#include "lighting.hpp"
#include <cstdio>

void lighting::upload(shader& s) const
{
  s.set_uniform("u_light_count", count_);

  char name[64];
  for (int i = 0; i < count_; ++i) {
    const light& l = lights_[i];

    std::snprintf(name, sizeof(name), "u_lights[%d].type_color", i);
    s.set_uniform(name,
      static_cast<float>(static_cast<int>(l.type)),
      l.color.x, l.color.y, l.color.z);

    std::snprintf(name, sizeof(name), "u_lights[%d].intensity_pos", i);
    s.set_uniform(name,
      l.intensity,
      l.position.x, l.position.y, l.position.z);

    std::snprintf(name, sizeof(name), "u_lights[%d].range_dir", i);
    s.set_uniform(name,
      l.range,
      l.direction.x, l.direction.y, l.direction.z);

    std::snprintf(name, sizeof(name), "u_lights[%d].spot_angles", i);
    s.set_uniform(name,
      l.spot_cos_inner, l.spot_cos_outer, 0.0f, 0.0f);
  }
}
