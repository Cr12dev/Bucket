#pragma once

#include <cmath>
#include <limits>

#include "core/math/mat4.hpp"
#include "core/math/vec3.hpp"
#include "core/math/vec4.hpp"

// Frustum culling: planes extracted from a view-projection matrix
// (Gribb-Hartmann), tested against world-space AABBs.
struct frustum {
  vec4 planes[6];  // left, right, bottom, top, near, far (normals point inward)

  static frustum from_view_proj(const mat4& vp) {
    // Gribb-Hartmann needs the matrix rows; GLM stores columns, so transpose.
    glm::mat4 t = glm::transpose(vp.m);
    frustum f;
    f.planes[0] = vec4(t[3] + t[0]);  // left
    f.planes[1] = vec4(t[3] - t[0]);  // right
    f.planes[2] = vec4(t[3] + t[1]);  // bottom
    f.planes[3] = vec4(t[3] - t[1]);  // top
    f.planes[4] = vec4(t[3] + t[2]);  // near
    f.planes[5] = vec4(t[3] - t[2]);  // far

    for (int i = 0; i < 6; ++i) {
      float len = std::sqrt(f.planes[i].x * f.planes[i].x +
                            f.planes[i].y * f.planes[i].y +
                            f.planes[i].z * f.planes[i].z);
      if (len > 0.0f) {
        f.planes[i].x /= len;
        f.planes[i].y /= len;
        f.planes[i].z /= len;
        f.planes[i].w /= len;
      }
    }
    return f;
  }

  // True if the AABB (center, half-extents) is (partially) inside.
  bool contains(const vec3& center, const vec3& half) const {
    for (int i = 0; i < 6; ++i) {
      const vec4& p = planes[i];
      float d = p.x * center.x + p.y * center.y + p.z * center.z + p.w;
      float r = std::abs(p.x) * half.x + std::abs(p.y) * half.y + std::abs(p.z) * half.z;
      if (d + r < 0.0f)
        return false;
    }
    return true;
  }
};

// Exact world-space AABB of a unit box transformed by `model`.
inline void aabb_from_model(const mat4& model, vec3& out_min, vec3& out_max) {
  out_min = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
  out_max = { std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest() };
  for (int i = 0; i < 8; ++i) {
    vec3 corner{
      (i & 1) ? 0.5f : -0.5f,
      (i & 2) ? 0.5f : -0.5f,
      (i & 4) ? 0.5f : -0.5f,
    };
    vec3 p = model.transform_point(corner);
    out_min.x = std::min(out_min.x, p.x);
    out_min.y = std::min(out_min.y, p.y);
    out_min.z = std::min(out_min.z, p.z);
    out_max.x = std::max(out_max.x, p.x);
    out_max.y = std::max(out_max.y, p.y);
    out_max.z = std::max(out_max.z, p.z);
  }
}
