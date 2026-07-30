#include "shader.hpp"

#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>
#include <glm/glm.hpp>

shader::shader(const std::string& vertex_path, const std::string& fragment_path)
{
  std::string vert_src = load_file(vertex_path);
  std::string frag_src = load_file(fragment_path);

  GLuint vert = compile(GL_VERTEX_SHADER, vert_src);
  GLuint frag = compile(GL_FRAGMENT_SHADER, frag_src);

  id_ = glCreateProgram();
  glAttachShader(id_, vert);
  glAttachShader(id_, frag);
  glLinkProgram(id_);

  GLint success;
  glGetProgramiv(id_, GL_LINK_STATUS, &success);
  if (!success)
  {
    std::vector<char> log(1024);
    glGetProgramInfoLog(id_, 1024, nullptr, log.data());
    std::cerr << "shader link error: " << log.data() << std::endl;
    glDeleteProgram(id_);
    id_ = 0;
  }

  glDeleteShader(vert);
  glDeleteShader(frag);
}

shader::~shader()
{
  if (id_)
    glDeleteProgram(id_);
}

shader::shader(shader&& other) noexcept
  : id_(other.id_), uniform_cache_(std::move(other.uniform_cache_))
{
  other.id_ = 0;
}

shader& shader::operator=(shader&& other) noexcept
{
  if (this != &other)
  {
    if (id_) glDeleteProgram(id_);
    id_ = other.id_;
    uniform_cache_ = std::move(other.uniform_cache_);
    other.id_ = 0;
  }
  return *this;
}

void shader::bind() const
{
  glUseProgram(id_);
}

void shader::unbind()
{
  glUseProgram(0);
}

void shader::set_uniform(const std::string& name, int value)
{
  glUniform1i(uniform_location(name), value);
}

void shader::set_uniform(const std::string& name, float value)
{
  glUniform1f(uniform_location(name), value);
}

void shader::set_uniform(const std::string& name, float v0, float v1, float v2, float v3)
{
  glUniform4f(uniform_location(name), v0, v1, v2, v3);
}

void shader::set_uniform(const std::string& name, const float* matrix4x4)
{
  glUniformMatrix4fv(uniform_location(name), 1, GL_FALSE, matrix4x4);
}

GLuint shader::compile(GLenum type, const std::string& source)
{
  GLuint id = glCreateShader(type);
  const char* src = source.c_str();
  glShaderSource(id, 1, &src, nullptr);
  glCompileShader(id);

  GLint success;
  glGetShaderiv(id, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    std::vector<char> log(1024);
    glGetShaderInfoLog(id, 1024, nullptr, log.data());
    std::cerr << "shader compile error: " << log.data() << std::endl;
    glDeleteShader(id);
    return 0;
  }

  return id;
}

std::string shader::load_file(const std::string& path)
{
  std::ifstream file(path);
  if (!file)
  {
    std::cerr << "failed to open shader: " << path << std::endl;
    return {};
  }
  std::stringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

int shader::uniform_location(const std::string& name)
{
  auto it = uniform_cache_.find(name);
  if (it != uniform_cache_.end())
    return it->second;

  int loc = glGetUniformLocation(id_, name.c_str());
  uniform_cache_[name] = loc;
  return loc;
}
