#pragma once

#include <cstdint>

struct entity {
  uint32_t id;
  uint32_t generation;

  bool operator==(const entity& other) const {
    return id == other.id && generation == other.generation;
  }

  bool operator!=(const entity& other) const {
    return !(*this == other);
  }

  explicit operator bool() const {
    return id != ~0u;
  }
};

inline entity null_entity() { return { ~0u, 0 }; }
