#pragma once

#include <glad.h>
#include <vector>
#include <cstddef>

enum class shader_data_type
{
  float_,
  float2,
  float3,
  float4,
  int_,
  mat4
};

struct buffer_element
{
  shader_data_type type;
  bool normalized = false;

  static GLenum to_gl_type(shader_data_type t);
  static int size(shader_data_type t);
  static int count(shader_data_type t);
};

class vertex_buffer_layout
{
public:
  vertex_buffer_layout() = default;
  void push(shader_data_type type, bool normalized = false);
  const std::vector<buffer_element>& elements() const { return elements_; }
  int stride() const { return stride_; }

private:
  std::vector<buffer_element> elements_;
  int stride_ = 0;
};

class vertex_buffer
{
public:
  vertex_buffer(const void* data, std::size_t size);
  ~vertex_buffer();

  vertex_buffer(vertex_buffer&&) noexcept;
  vertex_buffer& operator=(vertex_buffer&&) noexcept;
  vertex_buffer(const vertex_buffer&) = delete;

  void bind() const;
  static void unbind();

private:
  GLuint id_ = 0;
};

class index_buffer
{
public:
  index_buffer() = default;
  index_buffer(const unsigned int* data, int count);
  ~index_buffer();

  index_buffer(index_buffer&&) noexcept;
  index_buffer& operator=(index_buffer&&) noexcept;
  index_buffer(const index_buffer&) = delete;

  void bind() const;
  static void unbind();
  int count() const { return count_; }

private:
  GLuint id_ = 0;
  int count_ = 0;
};

class vertex_array
{
public:
  vertex_array();
  ~vertex_array();

  vertex_array(vertex_array&&) noexcept;
  vertex_array& operator=(vertex_array&&) noexcept;
  vertex_array(const vertex_array&) = delete;

  void bind() const;
  static void unbind();

  void add_vertex_buffer(vertex_buffer&& vb, const vertex_buffer_layout& layout);
  void set_index_buffer(index_buffer&& ib);

private:
  GLuint id_ = 0;
  std::vector<vertex_buffer> vbos_;
  index_buffer ibo_;
};
