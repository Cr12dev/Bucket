#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "renderer/texture.hpp"

// Texture cache: loads each file from disk once and shares the GL texture.
namespace texture_cache {

// Returns the cached texture for `path`, or nullptr for an empty path.
// Failed loads fall back to a shared white texture (printed once).
inline std::shared_ptr<texture> load(const std::string& path) {
  static std::unordered_map<std::string, std::shared_ptr<texture>> cache;
  static bool warned = false;

  if (path.empty())
    return nullptr;

  auto it = cache.find(path);
  if (it != cache.end())
    return it->second;

  auto tex = std::make_shared<texture>(path);
  if (!*tex) {
    if (!warned) {
      std::fprintf(stderr, "texture_cache: load failed for \"%s\", using white\n", path.c_str());
      warned = true;
    }
    static std::shared_ptr<texture> white = std::make_shared<texture>(texture::white());
    cache[path] = white;
    return white;
  }
  cache[path] = tex;
  return tex;
}

// Single shared white 1x1 texture used as placeholder for empty slots.
inline std::shared_ptr<texture> white() {
  static std::shared_ptr<texture> white = std::make_shared<texture>(texture::white());
  return white;
}

} // namespace texture_cache
