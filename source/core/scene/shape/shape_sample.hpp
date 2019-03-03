#ifndef SU_CORE_SCENE_SHAPE_SAMPLE_HPP
#define SU_CORE_SCENE_SHAPE_SAMPLE_HPP

#include "base/math/vector2.hpp"
#include "base/math/vector3.hpp"

namespace scene::shape {

struct Sample_to {
    float3 wi;
    float3 uvw;
    float  pdf;
    float  t;
};

struct Sample_from {
    float3 p;
    float3 dir;
    float2 uv;
    float2 xy;
    float  pdf;
};

}  // namespace scene::shape

#endif
