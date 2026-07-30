#pragma once

#include "entity.hpp"
#include <vector>
#include <cstdint>

struct component {};

class component_pool_base {
public:
  virtual ~component_pool_base() = default;
  virtual void remove_entity(entity e) = 0;
  virtual bool has_entity(entity e) const = 0;
};

template<typename T>
class component_pool : public component_pool_base {
public:
  T* get(entity e) {
    if (e.id >= sparse_.size()) return nullptr;
    int idx = sparse_[e.id];
    if (idx < 0 || idx >= size_) return nullptr;
    if (dense_entities_[idx].id != e.id) return nullptr;
    return &components_[idx];
  }

  const T* get(entity e) const {
    if (e.id >= sparse_.size()) return nullptr;
    int idx = sparse_[e.id];
    if (idx < 0 || idx >= size_) return nullptr;
    if (dense_entities_[idx].id != e.id) return nullptr;
    return &components_[idx];
  }

  template<typename... Args>
  T& add(entity e, Args&&... args) {
    if (e.id >= sparse_.size()) {
      sparse_.resize(e.id + 1, -1);
    }

    int idx = size_++;
    if (dense_entities_.size() < static_cast<size_t>(size_)) {
      dense_entities_.resize(size_);
      components_.resize(size_);
    }
    dense_entities_[idx] = e;
    components_[idx] = T(std::forward<Args>(args)...);
    sparse_[e.id] = idx;
    return components_[idx];
  }

  void remove_entity(entity e) override {
    if (e.id >= sparse_.size()) return;
    int idx = sparse_[e.id];
    if (idx < 0) return;

    int last = size_ - 1;
    entity last_entity = dense_entities_[last];
    dense_entities_[idx] = last_entity;
    components_[idx] = std::move(components_[last]);
    sparse_[last_entity.id] = idx;

    sparse_[e.id] = -1;
    --size_;
  }

  bool has_entity(entity e) const override {
    return get(e) != nullptr;
  }

  int size() const { return size_; }

  entity entity_at(int index) const {
    return dense_entities_[index];
  }

  T& component_at(int index) {
    return components_[index];
  }

  const T& component_at(int index) const {
    return components_[index];
  }

private:
  std::vector<int> sparse_;
  std::vector<entity> dense_entities_;
  std::vector<T> components_;
  int size_ = 0;
};
