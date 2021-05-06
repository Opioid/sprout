#ifndef SU_CORE_SCENE_SHAPE_SAMPLE_HPP
#define SU_CORE_SCENE_SHAPE_SAMPLE_HPP

#include "base/math/vector2.hpp"
#include "base/math/vector3.hpp"

namespace scene::shape {

struct Sample_to {
    inline Sample_to() = default;

    inline Sample_to(float3_p wi, float3_p n, float3_p uvw, float pdf, float t)
        : wi(wi, pdf), n(n, t), uvw(uvw){};

    inline float pdf() const {
        return wi[3];
    }

    inline float& pdf() {
        return wi[3];
    }

    inline float t() const {
        return n[3];
    }

    float3 wi;
    float3 n;
    float3 uvw;
};

struct Sample_from {
    inline Sample_from() = default;

    inline Sample_from(float3_p p, float3_p n, float3_p dir, float2 uv, float2 xy, float pdf)
        : p(p, pdf), n(n), dir(dir), uv(uv), xy(xy){};

    inline float pdf() const {
        return p[3];
    }

    inline float& pdf() {
        return p[3];
    }

    float3 p;
    float3 n;
    float3 dir;
    float2 uv;
    float2 xy;
};

}  // namespace scene::shape

#endif
