#pragma once

#include "core/window.hpp"
#include "core/timer.hpp"
#include "core/input.hpp"
#include <functional>
#include <memory>

class application {
public:
  using start_fn   = std::function<void()>;
  using update_fn  = std::function<void(double dt)>;
  using render_fn  = std::function<void()>;
  using cleanup_fn = std::function<void()>;

  application(const std::string& title, int width = 1280, int height = 720);
  ~application();

  void run(start_fn on_start,
           update_fn on_update,
           render_fn on_render,
           cleanup_fn on_cleanup = nullptr);

  window& get_window() { return *window_; }

private:
  std::unique_ptr<window> window_;
  timer timer_;
};
