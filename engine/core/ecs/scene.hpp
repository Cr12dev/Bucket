#pragma once

#include "entity.hpp"
#include "component.hpp"
#include "behaviour_component.hpp"
#include "components/tag.hpp"

#include <cstdio>
#include <cstring>
#include <memory>
#include <utility>
#include <vector>

namespace scene_io {

// Length-prefixed string helpers for component serialization.
inline void write_string(std::FILE* f, const std::string& s) {
  uint32_t len = static_cast<uint32_t>(s.size());
  std::fwrite(&len, sizeof(len), 1, f);
  std::fwrite(s.data(), 1, len, f);
}

inline void read_string(std::FILE* f, std::string& s) {
  uint32_t len = 0;
  std::fread(&len, sizeof(len), 1, f);
  s.resize(len);
  std::fread(s.data(), 1, len, f);
}

template<typename T>
inline void write_component(std::FILE* f, const T& c) {
  std::fwrite(&c, sizeof(T), 1, f);
}

template<>
inline void write_component<tag>(std::FILE* f, const tag& t) {
  uint32_t len = static_cast<uint32_t>(t.name.size());
  std::fwrite(&len, sizeof(len), 1, f);
  std::fwrite(t.name.data(), 1, len, f);
}

template<typename T>
inline void read_component(std::FILE* f, T& c) {
  std::fread(&c, sizeof(T), 1, f);
}

template<>
inline void read_component<tag>(std::FILE* f, tag& t) {
  uint32_t len = 0;
  std::fread(&len, sizeof(len), 1, f);
  t.name.resize(len);
  std::fread(t.name.data(), 1, len, f);
}
} // namespace scene_io

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

  // Creates an entity keeping a specific id (used when loading levels so
  // every object keeps the id it had when the map was saved).
  entity create_entity_with_id(uint32_t id) {
    if (generations_.size() <= id)
      generations_.resize(id + 1, 0);
    if (id >= next_id_)
      next_id_ = id + 1;
    return { id, 0 };
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
    while (pools_.size() <= id)
      pools_.push_back(nullptr);
    return static_cast<component_pool<T>*>(pools_[id].get());
  }

  template<typename T>
  const component_pool<T>* get_pool() const {
    auto id = component_id<T>();
    if (id >= pools_.size()) return nullptr;
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

  // -- serialization (.lev) -------------------------------------------
  // Saves every entity (with the listed component types) to a binary
  // level file. Non-POD components need a scene_io specialization.
  template<typename... Components>
  bool save(const char* path) const {
    std::FILE* f = std::fopen(path, "wb");
    if (!f) return false;

    const char magic[8] = { 'B', 'U', 'C', 'K', 'L', 'E', 'V', '3' };
    std::fwrite(magic, 1, sizeof(magic), f);

    uint32_t entity_count = next_id_;
    std::fwrite(&entity_count, sizeof(entity_count), 1, f);

    for (uint32_t id = 0; id < entity_count; ++id) {
      entity e{ id, 0 };

      uint32_t mask = 0;
      uint32_t bit = 0;
      ((mask |= (has_component<Components>(e) ? (1u << bit) : 0u), ++bit), ...);
      if (!mask) continue;

      std::fwrite(&id, sizeof(id), 1, f);
      std::fwrite(&mask, sizeof(mask), 1, f);

      bit = 0;
      ((has_component<Components>(e)
          ? scene_io::write_component<Components>(f, *get_component<Components>(e))
          : void(), ++bit), ...);
    }

    std::fclose(f);
    return true;
  }

  // Replaces the scene contents with the entities stored in the level
  // file. The load is transactional: on error the scene is left untouched.
  template<typename... Components>
  bool load(const char* path) {
    std::FILE* f = std::fopen(path, "rb");
    if (!f) return false;

    char magic[8];
    bool valid = std::fread(magic, 1, sizeof(magic), f) == sizeof(magic) &&
                 std::memcmp(magic, "BUCKLEV3", sizeof(magic)) == 0;
    if (!valid) {
      std::fclose(f);
      return false;
    }

    scene loaded;
    uint32_t entity_count = 0;
    if (std::fread(&entity_count, sizeof(entity_count), 1, f) != 1) {
      std::fclose(f);
      return false;
    }

    for (uint32_t i = 0; i < entity_count; ++i) {
      uint32_t id = 0;
      if (std::fread(&id, sizeof(id), 1, f) != 1) break;
      uint32_t mask = 0;
      if (std::fread(&mask, sizeof(mask), 1, f) != 1) break;
      if (!mask) continue;

      entity e = loaded.create_entity_with_id(id);
      uint32_t bit = 0;
      ((void)((mask & (1u << bit)) ? [&]() {
        Components tmp{};
        scene_io::read_component<Components>(f, tmp);
        loaded.add_component<Components>(e, std::move(tmp));
      }() : void(), ++bit), ...);
    }

    std::fclose(f);
    *this = std::move(loaded);
    return true;
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
