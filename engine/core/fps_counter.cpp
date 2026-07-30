#include "fps_counter.hpp"

fps_counter::fps_counter(double interval)
  : interval_(interval)
{
}

void fps_counter::tick(double dt)
{
  accum_ += dt;
  ++frames_;
  if (accum_ >= interval_) {
    fps_ = static_cast<int>(frames_ / accum_ + 0.5);
    accum_ = 0.0;
    frames_ = 0;
    if (callback_) {
      callback_(fps_);
    }
  }
}

void fps_counter::on_update(std::function<void(int)> callback)
{
  callback_ = std::move(callback);
}
