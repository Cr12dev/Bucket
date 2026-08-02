#pragma once

#include <cstdio>
#include <string>

#include "core/ecs/component.hpp"
#include "core/ecs/scene.hpp"
#include "core/math/vec3.hpp"
#include "core/math/vec4.hpp"

// Visual properties shared by every drawable box.
// Serialized by the scene_io specializations below (paths as length-prefixed strings).
struct paint : public component {
  vec4 color = vec4(1.0f);
  float reflectivity = 0.0f;  // 0 = matte, 1 = mirror
  float uv_scale = 0.0f;      // 0 = use mesh UVs, >0 = automatic triplanar UV
  vec3 emission_color = vec3(0.0f);  // multiplier applied to the emission map

  // Texture paths relative to the project root. Empty = no map.
  std::string albedo;     // base color
  std::string normal;     // normal map (R+G tangent, B height)
  std::string roughness;  // gloss map (R = roughness, 0 = glossy)
  std::string emission;   // emission map

  // Per-face albedo overrides for boxes: 0=front 1=back 2=right 3=left 4=top 5=bottom.
  // When set, that face ignores `albedo` and uses this texture instead.
  std::string face_albedo[6];
};

namespace scene_io {

inline void write_string(std::FILE* f, const std::string& s) {
  uint32_t len = static_cast<uint32_t>(s.size());
  std::fwrite(&len, sizeof(len), 1, f);
  std::fwrite(s.data(), 1, len, f);
}

inline void read_string(std::FILE* f, std::string& s) {
  uint32_t len = 0;
  std::fread(&len, sizeof(len), 1, f);
  s.resize(len);
  std::fread(s.data(), 1, len, f);
}

template<>
inline void write_component<paint>(std::FILE* f, const paint& p) {
  std::fwrite(&p.color, sizeof(p.color), 1, f);
  std::fwrite(&p.reflectivity, sizeof(p.reflectivity), 1, f);
  std::fwrite(&p.uv_scale, sizeof(p.uv_scale), 1, f);
  std::fwrite(&p.emission_color, sizeof(p.emission_color), 1, f);
  write_string(f, p.albedo);
  write_string(f, p.normal);
  write_string(f, p.roughness);
  write_string(f, p.emission);
  for (int i = 0; i < 6; ++i)
    write_string(f, p.face_albedo[i]);
}

template<>
inline void read_component<paint>(std::FILE* f, paint& p) {
  std::fread(&p.color, sizeof(p.color), 1, f);
  std::fread(&p.reflectivity, sizeof(p.reflectivity), 1, f);
  std::fread(&p.uv_scale, sizeof(p.uv_scale), 1, f);
  std::fread(&p.emission_color, sizeof(p.emission_color), 1, f);
  read_string(f, p.albedo);
  read_string(f, p.normal);
  read_string(f, p.roughness);
  read_string(f, p.emission);
  for (int i = 0; i < 6; ++i)
    read_string(f, p.face_albedo[i]);
}

} // namespace scene_io
