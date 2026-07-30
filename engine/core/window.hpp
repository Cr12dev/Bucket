#pragma once

#include <glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <string>
#include <functional>

class window {
public:
  window(int width, int height, const std::string& title);
  ~window();

  bool should_close() const;
  void swap_buffers();
  void poll_events();

  int width() const { return width_; }
  int height() const { return height_; }
  GLFWwindow* native() const { return handle_; }

private:
  int width_;
  int height_;
  GLFWwindow* handle_;
};
