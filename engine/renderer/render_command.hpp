#pragma once

#include "renderer/buffer.hpp"
#include "renderer/mesh.hpp"

class render_command {
public:
  static void set_clear_color(float r, float g, float b, float a);
  static void clear();
  static void set_viewport(int x, int y, int w, int h);
  static void draw_indexed(const mesh& m);
  static void draw_arrays(const vertex_array& va, int count);
};
