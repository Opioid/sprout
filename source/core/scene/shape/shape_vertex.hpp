#ifndef SU_CORE_SCENE_SHAPE_VERTEX_HPP
#define SU_CORE_SCENE_SHAPE_VERTEX_HPP

#include "base/math/vector2.hpp"
#include "base/math/vector3.hpp"

namespace scene::shape {

struct Vertex {
  packed_float3 p;
  packed_float3 n;
  packed_float3 t;
  float2 uv;
  float bitangent_sign;
};

}  // namespace scene::shape

#endif
