#ifndef SU_CORE_SCENE_SHAPE_SAMPLE_HPP
#define SU_CORE_SCENE_SHAPE_SAMPLE_HPP

#include "base/math/vector2.hpp"
#include "base/math/vector3.hpp"

namespace scene::shape {

struct Sample {
  float3 wi;
  float2 uv;
  float pdf;
  float t;
  float epsilon;
};

}  // namespace scene::shape

#endif
