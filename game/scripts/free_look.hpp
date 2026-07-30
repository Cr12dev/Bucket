#pragma once

#include <Buckit.hpp>

struct free_look : public behaviour {
  camera* cam = nullptr;

  explicit free_look(camera* c) : cam(c) {}

  void on_awake() override {
    cam->set_position({ 0.0f, 0.0f, 3.0f });
  }

  void on_update(float dt) override {
    if (!cam) return;

    float speed = 2.5f * dt;
    float rot_speed = 30.0f * dt;

    if (input::key_down(GLFW_KEY_W)) cam->move(cam->forward() * speed);
    if (input::key_down(GLFW_KEY_S)) cam->move(-cam->forward() * speed);
    if (input::key_down(GLFW_KEY_A)) cam->move(-cam->right() * speed);
    if (input::key_down(GLFW_KEY_D)) cam->move(cam->right() * speed);
    if (input::key_down(GLFW_KEY_SPACE)) cam->move(vec3::up() * speed);
    if (input::key_down(GLFW_KEY_LEFT_SHIFT)) cam->move(vec3::down() * speed);

    double mx = input::mouse_x();
    double my = input::mouse_y();
    if (first_mouse_) {
      last_mx_ = mx;
      last_my_ = my;
      first_mouse_ = false;
    }
    double dx = mx - last_mx_;
    double dy = my - last_my_;
    last_mx_ = mx;
    last_my_ = my;

    if (input::mouse_button_down(GLFW_MOUSE_BUTTON_RIGHT))
      cam->rotate(static_cast<float>(dx) * rot_speed,
                  static_cast<float>(-dy) * rot_speed);
  }

private:
  double last_mx_ = 0.0, last_my_ = 0.0;
  bool first_mouse_ = true;
};
