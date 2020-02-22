#ifndef SU_CORE_SCENE_SHAPE_SAMPLE_HPP
#define SU_CORE_SCENE_SHAPE_SAMPLE_HPP

#include "base/math/vector2.hpp"
#include "base/math/vector3.hpp"

namespace scene::shape {

struct Sample_to {
    inline Sample_to() = default;

    inline Sample_to(float3 const& wi, float3 const& uvw, float pdf, float t)
        : wi(wi[0], wi[1], wi[2], pdf), uvw(uvw[0], uvw[1], uvw[2], t){};

    inline float pdf() const {
        return wi[3];
    }

    inline float& pdf() {
        return wi[3];
    }

    inline float t() const {
        return uvw[3];
    }

    float3 wi;
    float3 uvw;
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
