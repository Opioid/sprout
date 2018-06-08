#ifndef SU_CORE_SCENE_ENTITY_COMPOSED_TRANSFORMATION_HPP
#define SU_CORE_SCENE_ENTITY_COMPOSED_TRANSFORMATION_HPP

#include "base/math/matrix3x3.hpp"
#include "base/math/matrix4x4.hpp"
#include "base/math/vector3.hpp"

namespace math {
struct Transformation;
}

namespace scene::entity {

struct Composed_transformation {
    void set(math::Transformation const& t);

    float3 world_to_object_point(f_float3 p) const;
    float3 world_to_object_vector(f_float3 v) const;

    float4x4 world_to_object;
    float4x4 object_to_world;
    float3x3 rotation;
    float3   position;
    float3   scale;
};

}  // namespace scene::entity

#endif
