#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_PRIMITIVE_MT_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_PRIMITIVE_MT_HPP

#include "base/math/vector4.hpp"

namespace scene::shape::triangle {

struct Shading_vertex_MTC {
    Shading_vertex_MTC();

    Shading_vertex_MTC(float3_p n, float3_p t, float2 uv);

    float4 n_u;
    float4 t_v;
};

}  // namespace scene::shape::triangle

#endif
