#pragma once

#include <cstdio>
#include <memory>
#include <string>
#include <unordered_map>

#include "renderer/shader.hpp"

// Shader registry: named shaders shared across the engine. Entities pick
// their material shader by id (see paint::shader).
namespace shader_cache {

inline void put(const std::string& name, std::shared_ptr<shader> s) {
  static std::unordered_map<std::string, std::shared_ptr<shader>> registry;
  registry[name] = std::move(s);
}

// Returns nullptr if the id was never registered (warns once).
inline std::shared_ptr<shader> get(const std::string& name) {
  static std::unordered_map<std::string, std::shared_ptr<shader>> registry;
  static bool warned = false;

  auto it = registry.find(name);
  if (it != registry.end())
    return it->second;
  if (!warned) {
    std::fprintf(stderr, "shader_cache: unknown shader \"%s\"\n", name.c_str());
    warned = true;
  }
  return nullptr;
}

} // namespace shader_cache
