#include "timer.hpp"

timer::timer()
  : last_(clock::now())
  , delta_(0.0)
{
}

void timer::tick()
{
  auto now = clock::now();
  auto elapsed = std::chrono::duration<double>(now - last_).count();
  delta_ = elapsed;
  last_ = now;
}
