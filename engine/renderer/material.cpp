#include "material.hpp"

material::material(std::shared_ptr<shader> shader)
  : shader_(std::move(shader))
{
}

void material::set_texture(const std::string& name, const texture* tex)
{
  int unit = static_cast<int>(textures_.size());
  textures_[name] = { tex, unit };
}

void material::set_color(const std::string& name, float r, float g, float b, float a)
{
  colors_[name] = { r, g, b, a };
}

void material::bind() const
{
  if (!shader_) return;
  shader_->bind();

  for (auto& [name, slot] : textures_) {
    slot.tex->bind(slot.unit);
    shader_->set_uniform(name, slot.unit);
  }

  for (auto& [name, color] : colors_) {
    shader_->set_uniform(name, color[0], color[1], color[2], color[3]);
  }
}
