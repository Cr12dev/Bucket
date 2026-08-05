#pragma once

#include <cstdio>
#include <string>

#include "core/ecs/component.hpp"
#include "core/ecs/scene.hpp"

// Selects which mesh a drawable entity renders with. The id maps to a
// mesh in mesh_cache (see renderer/mesh_cache.hpp). Serialized with a
// scene_io specialization so saved maps keep their mesh choice.
struct mesh_component : public component {
  std::string id = "cube";
};

namespace scene_io {
template<>
inline void write_component<mesh_component>(std::FILE* f, const mesh_component& m) {
  write_string(f, m.id);
}

template<>
inline void read_component<mesh_component>(std::FILE* f, mesh_component& m) {
  read_string(f, m.id);
}
} // namespace scene_io
