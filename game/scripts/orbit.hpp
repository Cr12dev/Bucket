#pragma once

#include <Buckit.hpp>

struct orbit : public behaviour {
  float speed = 0.5f;
  float elapsed = 0.0f;

  void on_awake() override {
    auto* t = get_scene()->get_component<transform>(get_entity());
    if (t) t->position = { 0.0f, 0.0f, 0.0f };
  }

  void on_update(float dt) override {
    elapsed += dt * speed;
    auto* t = get_scene()->get_component<transform>(get_entity());
    if (t) {
      t->position = {
        std::cos(elapsed) * 2.0f,
        std::sin(elapsed * 0.7f) * 0.5f,
        std::sin(elapsed) * 2.0f
      };
    }
  }
};
