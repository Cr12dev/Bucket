#pragma once

#include <cstdio>

#include "core/ecs/components/paint.hpp"
#include "renderer/shader.hpp"
#include "renderer/texture_cache.hpp"

// Centralized material upload: pushes the paint component (color, reflectivity,
// UV mode, albedo/normal/roughness/emission/metallic maps and per-face albedo
// overrides) into the given shader. Texture units: 0 albedo, 3 normal,
// 4 roughness, 5 emission, 12 metallic, 6..11 per-face albedo (1 shadow depth,
// 2 mirror are reserved).
namespace material_bind {

inline void bind(shader* s, const paint* p)
{
  vec4 color = p ? p->color : vec4(1.0f);
  s->set_uniform("u_color", color.x, color.y, color.z, color.w);
  s->set_uniform("u_reflectivity", p ? p->reflectivity : 0.0f);
  s->set_uniform("u_uv_scale", p ? p->uv_scale : 0.0f);

  std::shared_ptr<texture> albedo = p ? texture_cache::load(p->albedo) : nullptr;
  std::shared_ptr<texture> normal = p ? texture_cache::load(p->normal) : nullptr;
  std::shared_ptr<texture> rough  = p ? texture_cache::load(p->roughness) : nullptr;
  std::shared_ptr<texture> emiss  = p ? texture_cache::load(p->emission) : nullptr;
  std::shared_ptr<texture> white  = texture_cache::white();

  (albedo ? albedo : white)->bind(0);
  s->set_uniform("u_albedo", 0);

  (normal ? normal : white)->bind(3);
  s->set_uniform("u_normal_map", 3);
  s->set_uniform("u_normal_enabled", normal ? 1.0f : 0.0f);

  (rough ? rough : white)->bind(4);
  s->set_uniform("u_roughness_map", 4);
  s->set_uniform("u_roughness_enabled", rough ? 1.0f : 0.0f);

  (emiss ? emiss : white)->bind(5);
  s->set_uniform("u_emission_map", 5);
  s->set_uniform("u_emission_enabled", emiss ? 1.0f : 0.0f);
  s->set_uniform("u_emission_color",
                 p ? p->emission_color.x : 0.0f,
                 p ? p->emission_color.y : 0.0f,
                 p ? p->emission_color.z : 0.0f);

  // per-face albedo overrides (texture units 6..11)
  for (int i = 0; i < 6; ++i) {
    std::shared_ptr<texture> face = p ? texture_cache::load(p->face_albedo[i]) : nullptr;
    (face ? face : white)->bind(6 + i);
    char name[40];
    std::snprintf(name, sizeof(name), "u_face_albedo[%d]", i);
    s->set_uniform(name, 6 + i);
    std::snprintf(name, sizeof(name), "u_face_albedo_enabled[%d]", i);
    s->set_uniform(name, face ? 1.0f : 0.0f);
  }
}

} // namespace material_bind
