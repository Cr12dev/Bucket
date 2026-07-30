#pragma once

#include "core/ecs/component.hpp"
#include <string>

struct tag : public component {
  std::string name;

  tag() = default;
  explicit tag(std::string n) : name(std::move(n)) {}
};
