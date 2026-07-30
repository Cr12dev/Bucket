#include "render_command.hpp"

void render_command::set_clear_color(float r, float g, float b, float a)
{
  glClearColor(r, g, b, a);
}

void render_command::clear()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void render_command::set_viewport(int x, int y, int w, int h)
{
  glViewport(x, y, w, h);
}

void render_command::draw_indexed(const mesh& m)
{
  m.bind();
  glDrawElements(GL_TRIANGLES, m.index_count(), GL_UNSIGNED_INT, nullptr);
}

void render_command::draw_arrays(const vertex_array& va, int count)
{
  va.bind();
  glDrawArrays(GL_TRIANGLES, 0, count);
}
