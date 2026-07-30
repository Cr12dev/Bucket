#include "core/application.hpp"
#include "core/init.hpp"
#include "platform/platform.hpp"
#include <glad.h>
#include <cstdio>

application::application(const std::string& title, int width, int height)
{
  if (!engine::init()) return;

  window_ = std::make_unique<window>(width, height, title);
  if (!window_->native()) return;

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::fprintf(stderr, "application: gladLoadGLLoader failed\n");
    BUCKET_BREAK();
    return;
  }

  input::init(window_->native());
}

application::~application()
{
  window_.reset();
  engine::shutdown();
}

void application::run(start_fn on_start,
                       update_fn on_update,
                       render_fn on_render,
                       cleanup_fn on_cleanup)
{
  if (!window_ || !window_->native()) return;

  if (on_start) on_start();

  while (!window_->should_close())
  {
    timer_.tick();
    double dt = timer_.delta_seconds();

    if (on_update) on_update(dt);
    if (on_render) on_render();

    input::end_frame();
    window_->swap_buffers();
    window_->poll_events();
  }

  if (on_cleanup) on_cleanup();
}
