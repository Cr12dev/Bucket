#pragma once

#include "scene.hpp"

class entity_system {
public:
  virtual ~entity_system() = default;
  virtual void update(scene& s, float dt) = 0;
};

class system_manager {
public:
  template<typename T, typename... Args>
  T& add(Args&&... args) {
    auto sys = std::make_unique<T>(std::forward<Args>(args)...);
    auto* ptr = sys.get();
    systems_.push_back(std::move(sys));
    return *ptr;
  }

  void update(scene& s, float dt) {
    for (auto& sys : systems_)
      sys->update(s, dt);
  }

  void clear() { systems_.clear(); }

private:
  std::vector<std::unique_ptr<entity_system>> systems_;
};
