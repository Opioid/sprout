#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_ENCODING_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_ENCODING_HPP

#include "base/math/vector2.hpp"
#include "base/math/vector3.hpp"

namespace scene::shape::triangle {

struct Norm23x2_Sign1x2 {
    void decode(packed_float3& v, float& s) const;
    void encode(packed_float3 const& v, float s);

    uint16_t data[3];
};

struct Compressed_vertex {
    packed_float3    p;
    Norm23x2_Sign1x2 n;
    Norm23x2_Sign1x2 t;
    float2           uv;
};

}  // namespace scene::shape::triangle

#endif
