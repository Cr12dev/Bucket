#pragma once

#include <glad.h>
#include <string>
#include <unordered_map>

class shader {
public:
  shader() = default;
  shader(const std::string& vertex_path, const std::string& fragment_path);
  ~shader();

  shader(shader&&) noexcept;
  shader& operator=(shader&&) noexcept;

  shader(const shader&) = delete;
  shader& operator=(const shader&) = delete;

  void bind() const;
  static void unbind();

  GLuint id() const { return id_; }
  explicit operator bool() const { return id_ != 0; }

  void set_uniform(const std::string& name, int value);
  void set_uniform(const std::string& name, float value);
  void set_uniform(const std::string& name, float v0, float v1, float v2, float v3);
  void set_uniform(const std::string& name, const float* matrix4x4);

private:
  GLuint id_ = 0;
  std::unordered_map<std::string, int> uniform_cache_;

  static GLuint compile(GLenum type, const std::string& source);
  static std::string load_file(const std::string& path);
  int uniform_location(const std::string& name);
};
