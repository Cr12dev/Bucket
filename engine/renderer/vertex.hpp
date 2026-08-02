#pragma once

#include "core/math/vec3.hpp"
#include "core/math/vec2.hpp"
#include "renderer/buffer.hpp"

struct vertex {
  vec3 position;
  vec3 normal;
  vec2 uv;
  float face = 0.0f;  // face slot for per-face materials (cube: 0-5)

  static vertex_buffer_layout layout() {
    vertex_buffer_layout l;
    l.push(shader_data_type::float3);
    l.push(shader_data_type::float3);
    l.push(shader_data_type::float2);
    l.push(shader_data_type::float_);
    return l;
  }
};
