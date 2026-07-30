#pragma once

#include "entity.hpp"
#include "component.hpp"
#include "behaviour_component.hpp"
#include <memory>
#include <vector>

class scene {
public:
  entity create_entity() {
    entity e;
    if (!free_list_.empty()) {
      e = free_list_.back();
      free_list_.pop_back();
    } else {
      e = { next_id_++, 0 };
      if (generations_.size() <= e.id)
        generations_.resize(e.id + 1, 0);
    }
    return e;
  }

  void destroy_entity(entity e) {
    if (e.id >= generations_.size()) return;
    ++generations_[e.id];
    for (auto& pool : pools_)
      pool->remove_entity(e);
    free_list_.push_back(e);
  }

  bool is_alive(entity e) const {
    if (e.id >= generations_.size()) return false;
    return generations_[e.id] == e.generation;
  }

  template<typename T, typename... Args>
  T& add_component(entity e, Args&&... args) {
    auto* pool = get_or_create_pool<T>();
    return pool->add(e, std::forward<Args>(args)...);
  }

  template<typename T>
  T* get_component(entity e) {
    auto* pool = get_pool<T>();
    return pool ? pool->get(e) : nullptr;
  }

  template<typename T>
  const T* get_component(entity e) const {
    auto* pool = get_pool<T>();
    return pool ? pool->get(e) : nullptr;
  }

  template<typename T>
  bool has_component(entity e) const {
    auto* pool = get_pool<T>();
    return pool ? pool->has_entity(e) : false;
  }

  template<typename T>
  void remove_component(entity e) {
    auto* pool = get_pool<T>();
    if (pool) pool->remove_entity(e);
  }

  template<typename T>
  component_pool<T>* get_pool() {
    auto id = component_id<T>();
    return static_cast<component_pool<T>*>(pools_[id].get());
  }

  template<typename T>
  const component_pool<T>* get_pool() const {
    auto id = component_id<T>();
    return static_cast<const component_pool<T>*>(pools_[id].get());
  }

  template<typename T, typename F>
  void for_each(F&& callback) {
    auto* pool = get_pool<T>();
    if (!pool) return;
    for (int i = 0; i < pool->size(); ++i) {
      callback(pool->entity_at(i), pool->component_at(i));
    }
  }

  template<typename T, typename F>
  void for_each(F&& callback) const {
    auto* pool = get_pool<T>();
    if (!pool) return;
    for (int i = 0; i < pool->size(); ++i) {
      callback(pool->entity_at(i), pool->component_at(i));
    }
  }

  template<typename T1, typename T2, typename F>
  void for_each(F&& callback) {
    auto* p1 = get_pool<T1>();
    auto* p2 = get_pool<T2>();
    if (!p1 || !p2) return;
    for (int i = 0; i < p1->size(); ++i) {
      entity e = p1->entity_at(i);
      T2* c2 = p2->get(e);
      if (c2) callback(e, p1->component_at(i), *c2);
    }
  }

  void update(float dt) {
    for_each<behaviour_component>([&](entity e, behaviour_component& bc) {
      (void)e;
      bc.update_all(dt);
    });
  }

  template<typename T, typename... Args>
  T& add_behaviour(entity e, Args&&... args) {
    auto& bc = add_component<behaviour_component>(e);
    return bc.add<T>(e, this, std::forward<Args>(args)...);
  }

private:
  template<typename T>
  static uint32_t component_id() {
    static uint32_t id = next_component_id_++;
    return id;
  }

  template<typename T>
  component_pool<T>* get_or_create_pool() {
    auto id = component_id<T>();
    while (pools_.size() <= id)
      pools_.push_back(nullptr);
    if (!pools_[id])
      pools_[id] = std::make_unique<component_pool<T>>();
    return static_cast<component_pool<T>*>(pools_[id].get());
  }

  uint32_t next_id_ = 0;
  std::vector<uint32_t> generations_;
  std::vector<entity> free_list_;
  std::vector<std::unique_ptr<component_pool_base>> pools_;

  static uint32_t next_component_id_;
};
