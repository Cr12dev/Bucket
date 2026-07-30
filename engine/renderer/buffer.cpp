#include "buffer.hpp"

// --- buffer_element ---

GLenum buffer_element::to_gl_type(shader_data_type t)
{
  switch (t)
  {
    case shader_data_type::float_:  return GL_FLOAT;
    case shader_data_type::float2:  return GL_FLOAT;
    case shader_data_type::float3:  return GL_FLOAT;
    case shader_data_type::float4:  return GL_FLOAT;
    case shader_data_type::int_:    return GL_INT;
    case shader_data_type::mat4:    return GL_FLOAT;
  }
  return GL_FLOAT;
}

int buffer_element::size(shader_data_type t)
{
  switch (t)
  {
    case shader_data_type::float_:  return 4;
    case shader_data_type::float2:  return 8;
    case shader_data_type::float3:  return 12;
    case shader_data_type::float4:  return 16;
    case shader_data_type::int_:    return 4;
    case shader_data_type::mat4:    return 64;
  }
  return 0;
}

int buffer_element::count(shader_data_type t)
{
  switch (t)
  {
    case shader_data_type::float_:  return 1;
    case shader_data_type::float2:  return 2;
    case shader_data_type::float3:  return 3;
    case shader_data_type::float4:  return 4;
    case shader_data_type::int_:    return 1;
    case shader_data_type::mat4:    return 16;
  }
  return 0;
}

// --- vertex_buffer_layout ---

void vertex_buffer_layout::push(shader_data_type type, bool normalized)
{
  elements_.push_back({type, normalized});
  stride_ += buffer_element::size(type);
}

// --- vertex_buffer ---

vertex_buffer::vertex_buffer(const void* data, std::size_t size)
{
  glGenBuffers(1, &id_);
  glBindBuffer(GL_ARRAY_BUFFER, id_);
  glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

vertex_buffer::~vertex_buffer()
{
  if (id_) glDeleteBuffers(1, &id_);
}

vertex_buffer::vertex_buffer(vertex_buffer&& other) noexcept
  : id_(other.id_)
{
  other.id_ = 0;
}

vertex_buffer& vertex_buffer::operator=(vertex_buffer&& other) noexcept
{
  if (this != &other)
  {
    if (id_) glDeleteBuffers(1, &id_);
    id_ = other.id_;
    other.id_ = 0;
  }
  return *this;
}

void vertex_buffer::bind() const
{
  glBindBuffer(GL_ARRAY_BUFFER, id_);
}

void vertex_buffer::unbind()
{
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// --- index_buffer ---

index_buffer::index_buffer(const unsigned int* data, int count)
  : count_(count)
{
  glGenBuffers(1, &id_);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW);
}

index_buffer::~index_buffer()
{
  if (id_) glDeleteBuffers(1, &id_);
}

index_buffer::index_buffer(index_buffer&& other) noexcept
  : id_(other.id_), count_(other.count_)
{
  other.id_ = 0;
  other.count_ = 0;
}

index_buffer& index_buffer::operator=(index_buffer&& other) noexcept
{
  if (this != &other)
  {
    if (id_) glDeleteBuffers(1, &id_);
    id_ = other.id_;
    count_ = other.count_;
    other.id_ = 0;
    other.count_ = 0;
  }
  return *this;
}

void index_buffer::bind() const
{
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_);
}

void index_buffer::unbind()
{
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// --- vertex_array ---

vertex_array::vertex_array()
{
  glGenVertexArrays(1, &id_);
}

vertex_array::~vertex_array()
{
  if (id_) glDeleteVertexArrays(1, &id_);
}

vertex_array::vertex_array(vertex_array&& other) noexcept
  : id_(other.id_), vbos_(std::move(other.vbos_)), ibo_(std::move(other.ibo_))
{
  other.id_ = 0;
}

vertex_array& vertex_array::operator=(vertex_array&& other) noexcept
{
  if (this != &other)
  {
    if (id_) glDeleteVertexArrays(1, &id_);
    id_ = other.id_;
    vbos_ = std::move(other.vbos_);
    ibo_ = std::move(other.ibo_);
    other.id_ = 0;
  }
  return *this;
}

void vertex_array::bind() const
{
  glBindVertexArray(id_);
}

void vertex_array::unbind()
{
  glBindVertexArray(0);
}

void vertex_array::add_vertex_buffer(vertex_buffer&& vb, const vertex_buffer_layout& layout)
{
  bind();
  vb.bind();

  int offset = 0;
  for (std::size_t i = 0; i < layout.elements().size(); ++i)
  {
    const auto& elem = layout.elements()[i];
    glEnableVertexAttribArray(i);
    glVertexAttribPointer(
      i,
      buffer_element::count(elem.type),
      buffer_element::to_gl_type(elem.type),
      elem.normalized ? GL_TRUE : GL_FALSE,
      layout.stride(),
      reinterpret_cast<const void*>(offset)
    );
    offset += buffer_element::size(elem.type);
  }

  vbos_.push_back(std::move(vb));
}

void vertex_array::set_index_buffer(index_buffer&& ib)
{
  bind();
  ib.bind();
  ibo_ = std::move(ib);
}
