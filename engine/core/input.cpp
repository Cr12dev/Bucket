#include "input.hpp"

GLFWwindow* input::window_ = nullptr;
std::unordered_map<int, bool> input::keys_prev_;
double input::scroll_offset_ = 0.0;

void input::init(GLFWwindow* window)
{
  window_ = window;
  glfwSetScrollCallback(window, scroll_callback);
}

bool input::key_down(int key)
{
  return glfwGetKey(window_, key) == GLFW_PRESS;
}

bool input::key_pressed(int key)
{
  bool now = glfwGetKey(window_, key) == GLFW_PRESS;
  bool prev = keys_prev_[key];
  keys_prev_[key] = now;
  return now && !prev;
}

bool input::mouse_button_down(int button)
{
  return glfwGetMouseButton(window_, button) == GLFW_PRESS;
}

double input::mouse_x()
{
  double x, y;
  glfwGetCursorPos(window_, &x, &y);
  return x;
}

double input::mouse_y()
{
  double x, y;
  glfwGetCursorPos(window_, &x, &y);
  return y;
}

double input::scroll_y()
{
  return scroll_offset_;
}

void input::end_frame()
{
  scroll_offset_ = 0.0;
}

void input::scroll_callback(GLFWwindow* /*win*/, double /*xoffset*/, double yoffset)
{
  scroll_offset_ += yoffset;
}
