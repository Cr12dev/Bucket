#pragma once

#include "core/ecs/scene.hpp"
#include "core/ecs/components/transform.hpp"
#include "core/ecs/components/tag.hpp"
#include "core/ecs/components/paint.hpp"
#include "core/ecs/components/light.hpp"

// Engine-side prefabs: ready-made entities for the most common building
// blocks (geometry + lights). Each factory creates an entity with its
// components already configured and returns it.
namespace prefab {

// --- geometry ---------------------------------------------------------

// Generic box with a paint component. `reflectivity` makes it mirror-like
// when the scene has a planar mirror active. `uv_scale` enables automatic
// triplanar UVs (0 keeps the mesh UVs).
inline entity box(scene& s, const vec3& pos, const vec3& size,
                  const vec3& color, float reflectivity = 0.0f,
                  float uv_scale = 0.0f)
{
  entity e = s.create_entity();
  transform& t = s.add_component<transform>(e);
  t.position = pos;
  t.scale = size;
  paint& p = s.add_component<paint>(e);
  p.color = { color.x, color.y, color.z, 1.0f };
  p.reflectivity = reflectivity;
  p.uv_scale = uv_scale;
  return e;
}

inline entity floor(scene& s, const vec3& pos, const vec3& size,
                    const vec3& color, float reflectivity = 0.0f,
                    float uv_scale = 0.5f)
{
  return box(s, pos, size, color, reflectivity, uv_scale);
}

inline entity wall(scene& s, const vec3& pos, const vec3& size,
                   const vec3& color, float uv_scale = 0.5f)
{
  return box(s, pos, size, color, 0.0f, uv_scale);
}

inline entity crate(scene& s, const vec3& pos, float size = 2.0f,
                    const vec3& color = vec3(0.52f, 0.36f, 0.20f),
                    float uv_scale = 1.0f)
{
  return box(s, pos, vec3(size, size, size), color, 0.0f, uv_scale);
}

inline entity pillar(scene& s, const vec3& pos,
                     float width = 2.0f, float height = 8.0f,
                     const vec3& color = vec3(0.62f, 0.62f, 0.64f),
                     float uv_scale = 1.0f)
{
  return box(s, pos, vec3(width, height, width), color, 0.0f, uv_scale);
}

inline entity sandbag(scene& s, const vec3& pos,
                      const vec3& size = vec3(3.0f, 1.0f, 1.5f),
                      const vec3& color = vec3(0.66f, 0.58f, 0.42f),
                      float uv_scale = 1.0f)
{
  return box(s, pos, size, color, 0.0f, uv_scale);
}

// --- lights -----------------------------------------------------------

inline entity sun(scene& s, const vec3& direction,
                  const vec3& color = vec3(1.0f, 0.95f, 0.85f),
                  float intensity = 1.2f)
{
  entity e = s.create_entity();
  light& l = s.add_component<light>(e);
  l.type = light_type::directional;
  l.color = color;
  l.intensity = intensity;
  l.direction = direction;
  return e;
}

inline entity point_light(scene& s, const vec3& pos, const vec3& color,
                          float intensity, float range)
{
  entity e = s.create_entity();
  light& l = s.add_component<light>(e);
  l.type = light_type::point;
  l.color = color;
  l.intensity = intensity;
  l.position = pos;
  l.range = range;
  return e;
}

inline entity spot_light(scene& s, const vec3& pos, const vec3& direction,
                         const vec3& color, float intensity, float range,
                         float inner_deg, float outer_deg)
{
  entity e = s.create_entity();
  light& l = s.add_component<light>(e);
  l.type = light_type::spot;
  l.color = color;
  l.intensity = intensity;
  l.position = pos;
  l.direction = direction;
  l.range = range;
  l.spot_cos_inner = std::cos(inner_deg * 3.14159265f / 180.0f);
  l.spot_cos_outer = std::cos(outer_deg * 3.14159265f / 180.0f);
  return e;
}

} // namespace prefab
