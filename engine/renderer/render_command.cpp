#include "render_command.hpp"
#include <vector>

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

void render_command::draw_grid(int divisions, float spacing, const float* view_proj, const float* color)
{
  static const char* grid_vert_src =
    "#version 330 core\n"
    "layout (location = 0) in vec3 a_position;\n"
    "uniform mat4 u_view_proj;\n"
    "void main() { gl_Position = u_view_proj * vec4(a_position, 1.0); }\n";

  static const char* grid_frag_src =
    "#version 330 core\n"
    "uniform vec4 u_color;\n"
    "out vec4 frag_color;\n"
    "void main() { frag_color = u_color; }\n";

  static GLuint grid_program = 0;
  static GLint grid_vp_loc = -1;
  static GLint grid_color_loc = -1;

  static unsigned int grid_vao = 0;
  static unsigned int grid_vbo = 0;
  static int grid_count = 0;

  if (!grid_program) {
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &grid_vert_src, nullptr);
    glCompileShader(vs);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &grid_frag_src, nullptr);
    glCompileShader(fs);

    grid_program = glCreateProgram();
    glAttachShader(grid_program, vs);
    glAttachShader(grid_program, fs);
    glLinkProgram(grid_program);
    glDeleteShader(vs);
    glDeleteShader(fs);

    grid_vp_loc = glGetUniformLocation(grid_program, "u_view_proj");
    grid_color_loc = glGetUniformLocation(grid_program, "u_color");
  }

  if (!grid_vao) {
    float half = divisions * spacing * 0.5f;
    std::vector<float> verts;

    for (int i = 0; i <= divisions; ++i) {
      float t = -half + i * spacing;
      verts.push_back(t); verts.push_back(0.0f); verts.push_back(-half);
      verts.push_back(t); verts.push_back(0.0f); verts.push_back(half);
      verts.push_back(-half); verts.push_back(0.0f); verts.push_back(t);
      verts.push_back(half);  verts.push_back(0.0f); verts.push_back(t);
    }

    glGenVertexArrays(1, &grid_vao);
    glGenBuffers(1, &grid_vbo);

    glBindVertexArray(grid_vao);
    glBindBuffer(GL_ARRAY_BUFFER, grid_vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    grid_count = static_cast<int>(verts.size()) / 3;
  }

  glUseProgram(grid_program);
  glUniformMatrix4fv(grid_vp_loc, 1, GL_FALSE, view_proj);
  glUniform4fv(grid_color_loc, 1, color);

  glBindVertexArray(grid_vao);
  glDrawArrays(GL_LINES, 0, grid_count);
}
