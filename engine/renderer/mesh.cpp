#include "mesh.hpp"

mesh::mesh(const std::vector<vertex>& vertices, const std::vector<unsigned int>& indices)
  : index_count_(static_cast<int>(indices.size()))
{
  vao_ = vertex_array();
  vao_.add_vertex_buffer(
    vertex_buffer(vertices.data(), vertices.size() * sizeof(vertex)),
    vertex::layout()
  );
  vao_.set_index_buffer(
    index_buffer(indices.data(), index_count_)
  );
}

void mesh::bind() const
{
  vao_.bind();
}

mesh mesh::cube()
{
  // face slots: 0=front 1=back 2=right 3=left 4=top 5=bottom
  std::vector<vertex> verts = {
    // front
    {{-0.5f, -0.5f,  0.5f}, { 0,  0,  1}, {0, 0}, 0},
    {{ 0.5f, -0.5f,  0.5f}, { 0,  0,  1}, {1, 0}, 0},
    {{ 0.5f,  0.5f,  0.5f}, { 0,  0,  1}, {1, 1}, 0},
    {{-0.5f,  0.5f,  0.5f}, { 0,  0,  1}, {0, 1}, 0},
    // back
    {{ 0.5f, -0.5f, -0.5f}, { 0,  0, -1}, {0, 0}, 1},
    {{-0.5f, -0.5f, -0.5f}, { 0,  0, -1}, {1, 0}, 1},
    {{-0.5f,  0.5f, -0.5f}, { 0,  0, -1}, {1, 1}, 1},
    {{ 0.5f,  0.5f, -0.5f}, { 0,  0, -1}, {0, 1}, 1},
    // right
    {{ 0.5f, -0.5f,  0.5f}, { 1,  0,  0}, {0, 0}, 2},
    {{ 0.5f, -0.5f, -0.5f}, { 1,  0,  0}, {1, 0}, 2},
    {{ 0.5f,  0.5f, -0.5f}, { 1,  0,  0}, {1, 1}, 2},
    {{ 0.5f,  0.5f,  0.5f}, { 1,  0,  0}, {0, 1}, 2},
    // left
    {{-0.5f, -0.5f, -0.5f}, {-1,  0,  0}, {0, 0}, 3},
    {{-0.5f, -0.5f,  0.5f}, {-1,  0,  0}, {1, 0}, 3},
    {{-0.5f,  0.5f,  0.5f}, {-1,  0,  0}, {1, 1}, 3},
    {{-0.5f,  0.5f, -0.5f}, {-1,  0,  0}, {0, 1}, 3},
    // top
    {{-0.5f,  0.5f,  0.5f}, { 0,  1,  0}, {0, 0}, 4},
    {{ 0.5f,  0.5f,  0.5f}, { 0,  1,  0}, {1, 0}, 4},
    {{ 0.5f,  0.5f, -0.5f}, { 0,  1,  0}, {1, 1}, 4},
    {{-0.5f,  0.5f, -0.5f}, { 0,  1,  0}, {0, 1}, 4},
    // bottom
    {{-0.5f, -0.5f, -0.5f}, { 0, -1,  0}, {0, 0}, 5},
    {{ 0.5f, -0.5f, -0.5f}, { 0, -1,  0}, {1, 0}, 5},
    {{ 0.5f, -0.5f,  0.5f}, { 0, -1,  0}, {1, 1}, 5},
    {{-0.5f, -0.5f,  0.5f}, { 0, -1,  0}, {0, 1}, 5},
  };

  std::vector<unsigned int> idx = {
    0,1,2, 0,2,3,
    4,5,6, 4,6,7,
    8,9,10, 8,10,11,
    12,13,14, 12,14,15,
    16,17,18, 16,18,19,
    20,21,22, 20,22,23,
  };

  return mesh(verts, idx);
}

mesh mesh::quad()
{
  std::vector<vertex> verts = {
    {{-0.5f, -0.5f, 0.0f}, {0, 0, 1}, {0, 0}},
    {{ 0.5f, -0.5f, 0.0f}, {0, 0, 1}, {1, 0}},
    {{ 0.5f,  0.5f, 0.0f}, {0, 0, 1}, {1, 1}},
    {{-0.5f,  0.5f, 0.0f}, {0, 0, 1}, {0, 1}},
  };

  std::vector<unsigned int> idx = { 0,1,2, 0,2,3 };
  return mesh(verts, idx);
}

mesh mesh::fullscreen_quad()
{
  std::vector<vertex> verts = {
    {{-1.0f, -1.0f, 0.0f}, {0, 0, 1}, {0, 0}},
    {{ 1.0f, -1.0f, 0.0f}, {0, 0, 1}, {1, 0}},
    {{ 1.0f,  1.0f, 0.0f}, {0, 0, 1}, {1, 1}},
    {{-1.0f,  1.0f, 0.0f}, {0, 0, 1}, {0, 1}},
  };

  std::vector<unsigned int> idx = { 0,1,2, 0,2,3 };
  return mesh(verts, idx);
}
