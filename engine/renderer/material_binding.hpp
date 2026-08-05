#pragma once

#include "renderer/shader.hpp"
#include "renderer/texture.hpp"
#include <unordered_map>
#include <memory>
#include <array>

// Legacy renderer-side binding holder (superseded by material_bind + paint).
// The `material` name belongs to the paint alias used in game code.
class material_binding {
public:
  material_binding() = default;
  explicit material_binding(std::shared_ptr<shader> shader);

  void set_shader(std::shared_ptr<shader> s) { shader_ = std::move(s); }
  shader* get_shader() const { return shader_.get(); }

  void set_texture(const std::string& name, const texture* tex);
  void set_color(const std::string& name, float r, float g, float b, float a);

  void bind() const;

private:
  std::shared_ptr<shader> shader_;

  struct texture_slot {
    const texture* tex = nullptr;
    int unit = 0;
  };
  std::unordered_map<std::string, texture_slot> textures_;
  std::unordered_map<std::string, std::array<float, 4>> colors_;
};
