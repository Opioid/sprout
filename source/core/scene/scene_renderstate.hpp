#ifndef SU_CORE_SCENE_RENDERSTATE_HPP
#define SU_CORE_SCENE_RENDERSTATE_HPP

#include "base/math/vector3.inl"
#include "material/sampler_settings.hpp"

namespace scene {

struct Renderstate {
    using Filter = material::Sampler_settings::Filter;

    float3 tangent_to_world(float3_p v) const {
        return float3(v[0] * t[0] + v[1] * b[0] + v[2] * n[0],
                      v[0] * t[1] + v[1] * b[1] + v[2] * n[1],
                      v[0] * t[2] + v[1] * b[2] + v[2] * n[2]);
    }

    float3 tangent_to_world(float2 v) const {
        return float3(v[0] * t[0] + v[1] * b[0], v[0] * t[1] + v[1] * b[1],
                      v[0] * t[2] + v[1] * b[2]);
    }

    float ior() const {
        return p[3];
    }

    float alpha() const {
        return t[3];
    }

    float wavelength() const {
        return b[3];
    }

    float3 p;      // position in world space
    float3 geo_n;  // geometry normal in world space
    float3 t;      // interpolated tangent frame in world space
    float3 b;      // ...
    float3 n;      // ...
    float2 uv;     // texture coordinates

    uint32_t prop;
    uint32_t part;
    uint32_t primitive;
    uint32_t depth;

    uint64_t time;

    Filter filter;

    bool subsurface;
    bool avoid_caustics;
};

}  // namespace scene

#endif
