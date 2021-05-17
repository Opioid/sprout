#ifndef SU_CORE_SCENE_SHAPE_INTERSECTION_HPP
#define SU_CORE_SCENE_SHAPE_INTERSECTION_HPP

#include "base/math/vector2.hpp"
#include "base/math/vector3.hpp"

namespace scene::shape {

struct Intersection {
    float3 p;      // position in world space, or texture space, depending on context
    float3 geo_n;  // geometry normal in world space
    float3 t;      // interpolated tangent frame in world space
    float3 b;      // ...
    float3 n;      // ...
    float2 uv;     // texture coordinates

    uint32_t part;
    uint32_t primitive;
};

enum class Interpolation { All, No_tangent_space, Normal };

}  // namespace scene::shape

#endif
