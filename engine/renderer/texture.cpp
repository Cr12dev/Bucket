#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "texture.hpp"
#include <cstdio>
#include <vector>

texture::texture(const std::string& path)
{
  stbi_set_flip_vertically_on_load(true);

  int channels;
  unsigned char* data = stbi_load(path.c_str(), &w_, &h_, &channels, 4);
  if (!data) {
    std::fprintf(stderr, "texture: failed to load %s\n", path.c_str());
    return;
  }

  glGenTextures(1, &id_);
  glBindTexture(GL_TEXTURE_2D, id_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w_, h_, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  stbi_image_free(data);
}

texture::~texture()
{
  if (id_) glDeleteTextures(1, &id_);
}

texture::texture(texture&& other) noexcept
  : id_(other.id_), w_(other.w_), h_(other.h_)
{
  other.id_ = 0;
  other.w_ = other.h_ = 0;
}

texture& texture::operator=(texture&& other) noexcept
{
  if (this != &other) {
    if (id_) glDeleteTextures(1, &id_);
    id_ = other.id_;
    w_ = other.w_;
    h_ = other.h_;
    other.id_ = 0;
    other.w_ = other.h_ = 0;
  }
  return *this;
}

void texture::bind(int slot) const
{
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, id_);
}

void texture::unbind(int slot)
{
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, 0);
}

texture texture::checkerboard(int size, int checks)
{
  texture t;
  t.w_ = t.h_ = size;

  std::vector<unsigned char> pixels(size * size * 4);
  int cell = size / checks;

  for (int y = 0; y < size; ++y) {
    for (int x = 0; x < size; ++x) {
      int cx = x / cell;
      int cy = y / cell;
      bool white = (cx + cy) % 2 == 0;
      int i = (y * size + x) * 4;
      unsigned char v = white ? 255 : 60;
      pixels[i + 0] = v;
      pixels[i + 1] = v;
      pixels[i + 2] = v;
      pixels[i + 3] = 255;
    }
  }

  glGenTextures(1, &t.id_);
  glBindTexture(GL_TEXTURE_2D, t.id_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
  glGenerateMipmap(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  return t;
}
