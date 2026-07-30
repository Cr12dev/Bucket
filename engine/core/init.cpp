#include "init.hpp"
#include "platform/platform.hpp"

#include <glad.h>
#include <GLFW/glfw3.h>
#include <cstdio>

namespace engine {

bool init()
{
  if (!glfwInit()) {
    std::fprintf(stderr, "engine: glfwInit failed\n");
    BUCKET_BREAK();
    return false;
  }
  return true;
}

void shutdown()
{
  glfwTerminate();
}

}
