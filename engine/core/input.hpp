#pragma once

#include <GLFW/glfw3.h>
#include <unordered_map>

class input {
public:
  static void init(GLFWwindow* window);

  static bool key_down(int key);
  static bool key_pressed(int key);
  static bool mouse_button_down(int button);

  static double mouse_x();
  static double mouse_y();
  static double scroll_y();

  static void end_frame();

private:
  static GLFWwindow* window_;
  static std::unordered_map<int, bool> keys_prev_;
  static double scroll_offset_;

  static void scroll_callback(GLFWwindow* win, double xoffset, double yoffset);
};
