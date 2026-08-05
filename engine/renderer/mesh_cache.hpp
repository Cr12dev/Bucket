#pragma once

#include <cstdio>
#include <memory>
#include <string>
#include <unordered_map>

#include "renderer/mesh.hpp"

// Mesh registry: generates and caches the built-in meshes so every
// entity sharing a mesh id uses the same GPU resources.
namespace mesh_cache {

// Returns the cached mesh for `id`. Unknown ids fall back to the cube
// (and print a warning the first time).
inline std::shared_ptr<mesh> get(const std::string& id) {
  static std::unordered_map<std::string, std::shared_ptr<mesh>> cache;
  static bool warned = false;

  auto it = cache.find(id);
  if (it != cache.end())
    return it->second;

  std::shared_ptr<mesh> m;
  if (id == "cube") {
    m = std::make_shared<mesh>(mesh::cube());
  } else if (id == "quad") {
    m = std::make_shared<mesh>(mesh::quad());
  } else {
    if (!warned) {
      std::fprintf(stderr, "mesh_cache: unknown mesh \"%s\", using cube\n", id.c_str());
      warned = true;
    }
    m = std::make_shared<mesh>(mesh::cube());
  }
  cache[id] = m;
  return m;
}

inline std::shared_ptr<mesh> cube() { return get("cube"); }
inline std::shared_ptr<mesh> quad() { return get("quad"); }

} // namespace mesh_cache
