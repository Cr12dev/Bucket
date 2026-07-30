#pragma once

#include "core/ecs/entity.hpp"

class scene;

class behaviour {
public:
  virtual ~behaviour() = default;

  virtual void on_awake() {}
  virtual void on_start() {}
  virtual void on_update(float dt) {}

  entity get_entity() const { return owner_; }
  scene* get_scene() const { return scene_; }

private:
  friend class behaviour_component;
  entity owner_ = null_entity();
  scene* scene_ = nullptr;
  bool started_ = false;
};
