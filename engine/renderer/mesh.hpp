#pragma once

#include "renderer/vertex.hpp"
#include "renderer/buffer.hpp"
#include <vector>
#include <cstddef>

class mesh {
public:
  mesh() = default;
  mesh(const std::vector<vertex>& vertices, const std::vector<unsigned int>& indices);

  void bind() const;
  int index_count() const { return index_count_; }
  bool valid() const { return index_count_ > 0; }

  static mesh cube();
  static mesh quad();

private:
  vertex_array vao_;
  int index_count_ = 0;
};
