#pragma once

#include <Buckit.hpp>
#include <GLFW/glfw3.h>

// FPS controller: mouse look, WASD + sprint, jump with gravity and AABB
// collision against the scene's boxes (axis-aligned, sliding resolution).
struct fps_player : public behaviour {
  camera* cam = nullptr;
  scene* world = nullptr;
  GLFWwindow* window = nullptr;
  vec3 spawn = vec3(0.0f, 1.7f, 0.0f);

  bool mouse_captured = false;
  bool first_mouse_ = true;
  double last_mx_ = 0.0, last_my_ = 0.0;

  vec3 velocity = vec3(0.0f);
  bool on_ground = false;

  float eye_height = 1.7f;
  float half_w = 0.35f;   // half of width/depth
  float half_h = 0.9f;    // half of height

  float move_speed = 4.5f;
  float run_speed = 7.0f;
  float gravity = -22.0f;
  float jump_speed = 7.5f;
  float mouse_sens = 0.1f;

  fps_player(camera* c, scene* s, GLFWwindow* w, const vec3& spawn_pos)
    : cam(c), world(s), window(w), spawn(spawn_pos)
  {
  }

  void capture_mouse(bool capture)
  {
    mouse_captured = capture;
    glfwSetInputMode(window, GLFW_CURSOR,
                     capture ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    first_mouse_ = true;
  }

  void on_awake() override
  {
    cam->set_position(spawn);
    cam->set_yaw(-90.0f);
    cam->set_pitch(0.0f);
    capture_mouse(true);
  }

  // horizontal footprint overlap (ignores height)
  bool overlaps_xz(const vec3& feet, const transform& t) const
  {
    vec3 c = t.position;
    vec3 h = t.scale * 0.5f;
    return feet.x + half_w > c.x - h.x &&
           feet.x - half_w < c.x + h.x &&
           feet.z + half_w > c.z - h.z &&
           feet.z - half_w < c.z + h.z;
  }

  // full AABB overlap (player vs box). The bottom check uses a tiny
  // epsilon because walls/crates start exactly at floor level (y = 0).
  bool overlaps_box(const vec3& feet, const transform& t) const
  {
    vec3 c = t.position;
    vec3 h = t.scale * 0.5f;
    const float eps = 0.001f;
    return overlaps_xz(feet, t) &&
           feet.y + eps > c.y - h.y &&
           feet.y + half_h * 2.0f < c.y + h.y;
  }

  bool hit(const vec3& feet) const
  {
    bool collided = false;
    world->for_each<transform>([&](entity e, transform& t) {
      if (e != get_entity() && overlaps_box(feet, t)) collided = true;
    });
    return collided;
  }

  void on_update(float dt) override
  {
    if (dt > 0.05f) dt = 0.05f;  // clamp after heavy frames

    // --- mouse look ---
    double mx = input::mouse_x();
    double my = input::mouse_y();
    if (mouse_captured) {
      if (first_mouse_) {
        last_mx_ = mx;
        last_my_ = my;
        first_mouse_ = false;
      }
      double dx = mx - last_mx_;
      double dy = my - last_my_;
      last_mx_ = mx;
      last_my_ = my;
      cam->rotate(static_cast<float>(dx) * mouse_sens,
                  static_cast<float>(-dy) * mouse_sens);
    }

    if (input::key_pressed(GLFW_KEY_F1)) capture_mouse(!mouse_captured);
    if (input::key_pressed(GLFW_KEY_ESCAPE)) capture_mouse(false);

    vec3 feet = cam->position() - vec3(0.0f, eye_height, 0.0f);

    // --- horizontal movement ---
    float speed = input::key_down(GLFW_KEY_LEFT_SHIFT) ? run_speed : move_speed;
    vec3 wish(0.0f);
    if (input::key_down(GLFW_KEY_W)) wish += cam->forward();
    if (input::key_down(GLFW_KEY_S)) wish -= cam->forward();
    if (input::key_down(GLFW_KEY_A)) wish -= cam->right();
    if (input::key_down(GLFW_KEY_D)) wish += cam->right();
    wish.y = 0.0f;
    if (wish.x != 0.0f || wish.z != 0.0f) wish = wish.normalized();
    velocity.x = wish.x * speed;
    velocity.z = wish.z * speed;

    // --- jump / gravity ---
    if (mouse_captured && input::key_pressed(GLFW_KEY_SPACE) && on_ground) {
      velocity.y = jump_speed;
    }
    velocity.y += gravity * dt;
    if (velocity.y < -30.0f) velocity.y = -30.0f;

    vec3 disp = velocity * dt;

    // --- axis-separated collision (slide along walls) ---
    vec3 p = feet;
    p.x += disp.x;
    if (!hit(p)) { feet.x = p.x; } else { velocity.x = 0.0f; }

    p = feet;
    p.z += disp.z;
    if (!hit(p)) { feet.z = p.z; } else { velocity.z = 0.0f; }

    on_ground = false;
    p = feet;
    p.y += disp.y;
    if (disp.y <= 0.0f) {
      // falling: land on the highest box top our feet cross this frame
      float top = feet.y;
      bool landed = false;
      world->for_each<transform>([&](entity e, transform& t) {
        if (e == get_entity()) return;
        if (!overlaps_xz(p, t)) return;
        float t_top = t.position.y + t.scale.y * 0.5f;
        if (p.y <= t_top + 0.001f && p.y + half_h * 2.0f >= t_top) {
          if (t_top > top) {
            top = t_top;
            landed = true;
          }
        }
      });
      if (landed) {
        feet.y = top;
        velocity.y = 0.0f;
        on_ground = true;
      } else {
        feet.y = p.y;
      }
    } else {
      // rising: bump our head only if it crosses a box's bottom this frame
      float head_prev = feet.y + half_h * 2.0f;
      float head_new = p.y + half_h * 2.0f;
      bool bumped = false;
      world->for_each<transform>([&](entity e, transform& t) {
        if (e == get_entity()) return;
        if (!overlaps_xz(p, t)) return;
        float t_bottom = t.position.y - t.scale.y * 0.5f;
        if (head_prev <= t_bottom + 0.001f && head_new >= t_bottom) {
          bumped = true;
        }
      });
      if (bumped) {
        velocity.y = 0.0f;  // head bump
      } else {
        feet.y = p.y;
      }
    }

    // safety net: fell out of the world, respawn
    if (feet.y < -15.0f) {
      feet = spawn - vec3(0.0f, eye_height, 0.0f);
      velocity = vec3(0.0f);
      cam->set_yaw(-90.0f);
      cam->set_pitch(0.0f);
    }

    cam->set_position(feet + vec3(0.0f, eye_height, 0.0f));
  }
};
