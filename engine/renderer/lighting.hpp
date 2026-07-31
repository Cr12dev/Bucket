#pragma once

#include "core/ecs/components/light.hpp"
#include "shader.hpp"
#include <array>

class lighting {
public:
  static constexpr int max_lights = 8;

  void clear() { count_ = 0; }
  void add(const light& l) {
    if (count_ < max_lights) {
      lights_[count_++] = l;
    }
  }
  int count() const { return count_; }

  void upload(shader& s) const;

private:
  std::array<light, max_lights> lights_;
  int count_ = 0;
};
