#pragma once

#include "core/ecs/behaviour.hpp"
#include <memory>
#include <vector>

class behaviour_component : public component {
public:
  template<typename T, typename... Args>
  T& add(entity owner, scene* scn, Args&&... args) {
    auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
    auto* raw = ptr.get();
    raw->owner_ = owner;
    raw->scene_ = scn;
    raw->on_awake();
    behaviours_.push_back(std::move(ptr));
    return *raw;
  }

  void update_all(float dt) {
    if (!started_) {
      for (auto& b : behaviours_) {
        if (!b->started_) {
          b->started_ = true;
          b->on_start();
        }
      }
      started_ = true;
    }
    for (auto& b : behaviours_) {
      b->on_update(dt);
    }
  }

  template<typename T>
  T* get_behaviour() {
    for (auto& b : behaviours_) {
      auto* casted = dynamic_cast<T*>(b.get());
      if (casted) return casted;
    }
    return nullptr;
  }

  int count() const { return static_cast<int>(behaviours_.size()); }

private:
  std::vector<std::unique_ptr<behaviour>> behaviours_;
  bool started_ = false;
};
