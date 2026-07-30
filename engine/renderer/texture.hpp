#pragma once

#include <glad.h>
#include <string>

class texture {
public:
  texture() = default;
  explicit texture(const std::string& path);
  ~texture();

  texture(texture&&) noexcept;
  texture& operator=(texture&&) noexcept;
  texture(const texture&) = delete;

  void bind(int slot = 0) const;
  static void unbind(int slot = 0);

  explicit operator bool() const { return id_ != 0; }
  int width() const { return w_; }
  int height() const { return h_; }

  static texture checkerboard(int size = 64, int checks = 8);

private:
  GLuint id_ = 0;
  int w_ = 0, h_ = 0;
};
