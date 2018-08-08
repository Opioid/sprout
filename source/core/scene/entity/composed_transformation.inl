#ifndef SU_CORE_SCENE_ENTITY_COMPOSED_TRANSFORMATION_INL
#define SU_CORE_SCENE_ENTITY_COMPOSED_TRANSFORMATION_INL

#include "base/math/matrix4x4.inl"
#include "base/math/quaternion.inl"
#include "base/math/transformation.inl"
#include "composed_transformation.hpp"

namespace scene::entity {

inline void Composed_transformation::set(math::Transformation const& t) noexcept {
    float3x3 const rot = math::quaternion::create_matrix3x3(t.rotation);

    float4x4 const otw = math::compose(rot, t.scale, t.position);

    world_to_object = math::affine_inverted(otw);
    object_to_world = otw;

    rotation = rot;
    position = t.position;
    scale    = t.scale;
}

inline float3 Composed_transformation::world_to_object_point(float3 const& p) const noexcept {
    return math::transform_point(world_to_object, p);
}

inline float3 Composed_transformation::world_to_object_vector(float3 const& v) const noexcept {
    return math::transform_vector(world_to_object, v);
}

}  // namespace scene::entity

#endif
