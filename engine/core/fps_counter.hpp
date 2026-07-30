#pragma once

#include <functional>

class fps_counter {
public:
  fps_counter(double interval = 0.5);

  void tick(double dt);
  int fps() const { return fps_; }
  void on_update(std::function<void(int)> callback);

private:
  double interval_;
  double accum_ = 0.0;
  int frames_ = 0;
  int fps_ = 0;
  std::function<void(int)> callback_;
};
