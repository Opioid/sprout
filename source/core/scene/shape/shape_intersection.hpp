#ifndef SU_CORE_SCENE_SHAPE_INTERSECTION_HPP
#define SU_CORE_SCENE_SHAPE_INTERSECTION_HPP

#include "base/math/vector2.hpp"
#include "base/math/vector3.hpp"

namespace scene::shape {

struct Intersection {
    union {
        float3 p;    // position in world space
        float3 uvw;  // position in texture space
    };

    float3 geo_n;    // geometry normal in world space
    float3 t, b, n;  // interpolated tangent frame in world space
    float2 uv;       // texture coordinates

    uint32_t part;
};

struct Normals {
    float3 geo_n;  // geometry normal in world space
    float3 n;      // interpolated tangent frame in world space
};

enum class Visibility { None, Complete, Partial };

}  // namespace scene::shape

#endif
