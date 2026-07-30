#include "window.hpp"
#include "platform/platform.hpp"
#include <cstdio>

window::window(int width, int height, const std::string& title)
  : width_(width)
  , height_(height)
{
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef BUCKET_PLATFORM_MACOS
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  handle_ = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
  if (!handle_) {
    std::fprintf(stderr, "window: glfwCreateWindow failed\n");
    BUCKET_BREAK();
    return;
  }

  glfwMakeContextCurrent(handle_);
  glfwSwapInterval(1);
}

window::~window()
{
  if (handle_)
    glfwDestroyWindow(handle_);
}

bool window::should_close() const
{
  return glfwWindowShouldClose(handle_);
}

void window::swap_buffers()
{
  glfwSwapBuffers(handle_);
}

void window::poll_events()
{
  glfwPollEvents();
}
