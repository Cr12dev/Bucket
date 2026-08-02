#pragma once

#include "core/ecs/scene.hpp"
#include "core/ecs/components/transform.hpp"
#include "core/math/vec3.hpp"

#include <algorithm>
#include <limits>

// World-space raycast against the scene's axis-aligned boxes (each entity
// with a transform component is a solid AABB centered at transform.position
// with half-extents transform.scale / 2). Uses the slab method.
struct raycast_hit {
  bool hit = false;
  entity entity_hit = null_entity();
  float t = 0.0f;          // distance along the ray
  vec3 point = vec3(0.0f); // world-space hit point
  vec3 normal = vec3(0.0f, 1.0f, 0.0f);  // face normal
};

inline raycast_hit raycast_scene(scene& s, const vec3& origin,
                                 const vec3& dir, float max_dist = 500.0f,
                                 entity ignore = null_entity())
{
  const float inf = std::numeric_limits<float>::infinity();
  raycast_hit best;
  best.t = max_dist;

  s.for_each<transform>([&](entity e, transform& t) {
    if (e == ignore) return;

    vec3 c = t.position;
    vec3 h = t.scale * 0.5f;

    // slab entry/exit times per axis, with parallel-ray handling
    auto slab = [&](float min, float max, float o, float d) {
      if (d != 0.0f) {
        float inv = 1.0f / d;
        float t0 = (min - o) * inv;
        float t1 = (max - o) * inv;
        return std::make_pair(std::min(t0, t1), std::max(t0, t1));
      }
      bool inside = o >= min && o <= max;
      return std::make_pair(inside ? -inf : inf, inside ? inf : -inf);
    };

    auto [ex, exx] = slab(c.x - h.x, c.x + h.x, origin.x, dir.x);
    auto [ey, eyy] = slab(c.y - h.y, c.y + h.y, origin.y, dir.y);
    auto [ez, ezz] = slab(c.z - h.z, c.z + h.z, origin.z, dir.z);

    float t_enter = std::max({ ex, ey, ez });
    float t_exit = std::min({ exx, eyy, ezz });

    if (t_exit < 0.0f || t_exit < t_enter || t_enter > best.t) return;
    if (t_enter < 0.0f) t_enter = 0.0f;  // ray starts inside the box

    best.hit = true;
    best.entity_hit = e;
    best.t = t_enter;
    best.point = origin + dir * t_enter;

    // face normal: the axis that entered last wins
    if (t_enter == ex) {
      best.normal = { dir.x > 0.0f ? -1.0f : 1.0f, 0.0f, 0.0f };
    } else if (t_enter == ey) {
      best.normal = { 0.0f, dir.y > 0.0f ? -1.0f : 1.0f, 0.0f };
    } else {
      best.normal = { 0.0f, 0.0f, dir.z > 0.0f ? -1.0f : 1.0f };
    }
  });

  return best;
}
