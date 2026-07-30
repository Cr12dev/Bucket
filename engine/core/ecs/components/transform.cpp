#include "transform.hpp"
#include "core/math/transform.hpp"

mat4 transform::matrix() const
{
  return make_transform(position, rotation, scale);
}
