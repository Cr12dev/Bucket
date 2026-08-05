#pragma once

#include <functional>
#include <string>
#include <vector>

// Explicit render pipeline: the game configures an ordered list of named
// passes (e.g. mirror, shadows, main, crosshair) and run() executes them
// every frame. Each pass is responsible for binding its own target.
// This decouples the renderer from the game loop's render() body.
struct render_pass {
  std::string name;
  std::function<void()> execute;
};

class render_pipeline {
public:
  void add(const std::string& name, std::function<void()> execute) {
    passes_.push_back({ name, std::move(execute) });
  }

  void clear() { passes_.clear(); }

  void run() const {
    for (const render_pass& p : passes_)
      p.execute();
  }

  const std::vector<render_pass>& passes() const { return passes_; }

private:
  std::vector<render_pass> passes_;
};
