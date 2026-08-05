#pragma once

#include "core/ecs/scene.hpp"
#include "core/ecs/components/transform.hpp"
#include "core/ecs/components/tag.hpp"
#include "core/ecs/components/paint.hpp"
#include "core/ecs/components/light.hpp"

// Engine-side prefabs: ready-made entities for the most common building
// blocks (geometry + lights). Every prefab takes a mandatory unique `id`
// string (stored in a tag component) so game code can later find and edit
// the object from update() by its id.
namespace prefab {

// --- geometry ---------------------------------------------------------

// Generic box with a paint component. `reflectivity` makes it mirror-like
// when the scene has a planar mirror active.
inline entity box(scene& s, const vec3& pos, const vec3& size,
                  const vec3& color, const std::string& id)
{
  entity e = s.create_entity();
  transform& t = s.add_component<transform>(e);
  t.position = pos;
  t.scale = size;
  s.add_component<tag>(e, id);
  paint& p = s.add_component<paint>(e);
  p.color = { color.x, color.y, color.z, 1.0f };
  return e;
}

inline entity floor(scene& s, const vec3& pos, const vec3& size,
                    const vec3& color, float reflectivity, const std::string& id)
{
  entity e = box(s, pos, size, color, id);
  paint& p = *s.get_component<paint>(e);
  p.reflectivity = reflectivity;
  p.uv_scale = 0.5f;  // tile textures across the floor
  return e;
}

inline entity wall(scene& s, const vec3& pos, const vec3& size,
                   const vec3& color, const std::string& id)
{
  entity e = box(s, pos, size, color, id);
  paint& p = *s.get_component<paint>(e);
  p.uv_scale = 0.5f;  // triplanar tiling for big walls
  return e;
}

inline entity crate(scene& s, const vec3& pos, float size,
                    const vec3& color, const std::string& id)
{
  entity e = box(s, pos, vec3(size, size, size), color, id);
  paint& p = *s.get_component<paint>(e);
  p.uv_scale = 1.0f;
  return e;
}

inline entity pillar(scene& s, const vec3& pos,
                     float width, float height,
                     const vec3& color, const std::string& id)
{
  entity e = box(s, pos, vec3(width, height, width), color, id);
  paint& p = *s.get_component<paint>(e);
  p.uv_scale = 1.0f;
  return e;
}

inline entity sandbag(scene& s, const vec3& pos,
                      const vec3& size, const vec3& color, const std::string& id)
{
  entity e = box(s, pos, size, color, id);
  paint& p = *s.get_component<paint>(e);
  p.uv_scale = 1.0f;
  return e;
}

// --- lights -----------------------------------------------------------

inline entity sun(scene& s, const vec3& direction,
                  const vec3& color, float intensity, const std::string& id)
{
  entity e = s.create_entity();
  s.add_component<tag>(e, id);
  light& l = s.add_component<light>(e);
  l.type = light_type::directional;
  l.color = color;
  l.intensity = intensity;
  l.direction = direction;
  return e;
}

inline entity point_light(scene& s, const vec3& pos, const vec3& color,
                          float intensity, float range, const std::string& id)
{
  entity e = s.create_entity();
  s.add_component<tag>(e, id);
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
                         float inner_deg, float outer_deg, const std::string& id)
{
  entity e = s.create_entity();
  s.add_component<tag>(e, id);
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
