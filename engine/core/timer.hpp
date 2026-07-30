#pragma once

#include <chrono>

class timer {
public:
  timer();

  void tick();
  double delta_seconds() const { return delta_; }
  float delta_ms() const { return static_cast<float>(delta_ * 1000.0); }

private:
  using clock = std::chrono::high_resolution_clock;
  clock::time_point last_;
  double delta_;
};
