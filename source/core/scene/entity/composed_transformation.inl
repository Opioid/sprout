#ifndef SU_CORE_SCENE_ENTITY_COMPOSED_TRANSFORMATION_INL
#define SU_CORE_SCENE_ENTITY_COMPOSED_TRANSFORMATION_INL

#include "base/math/matrix4x4.inl"
#include "base/math/quaternion.inl"
#include "base/math/transformation.inl"
#include "composed_transformation.hpp"

namespace scene::entity {

inline void Composed_transformation::set(math::Transformation const& t) noexcept {
    float3x3 const rot = quaternion::create_matrix3x3(t.rotation);

    float4x4 const otw = compose(rot, t.scale, t.position);

    world_to_object = affine_inverted(otw);

    rotation = rot;

    rotation.r[0][3] = t.scale[0];
    rotation.r[1][3] = t.scale[1];
    rotation.r[2][3] = t.scale[2];

    position = t.position;
}

inline float Composed_transformation::scale_x() const noexcept {
    return rotation.r[0][3];
}

inline float Composed_transformation::scale_y() const noexcept {
    return rotation.r[1][3];
}

inline float Composed_transformation::scale_z() const noexcept {
    return rotation.r[2][3];
}

inline float2 Composed_transformation::scale_xy() const noexcept {
    return float2(rotation.r[0][3], rotation.r[1][3]);
}

inline float3 Composed_transformation::scale() const noexcept {
    return float3(rotation.r[0][3], rotation.r[1][3], rotation.r[2][3]);
}

inline float4x4 Composed_transformation::object_to_world() const noexcept {
    float3 const s(rotation.r[0][3], rotation.r[1][3], rotation.r[2][3]);
    return compose(rotation, s, position);
}

inline float3 Composed_transformation::world_to_object_point(float3 const& p) const noexcept {
    return transform_point(world_to_object, p);
}

inline float3 Composed_transformation::world_to_object_vector(float3 const& v) const noexcept {
    return transform_vector(world_to_object, v);
}

inline float3 Composed_transformation::object_to_world_point(float3 const& v) const noexcept {
    return Vector3f_a(
        rotation.r[0][3] *
                (v[0] * rotation.r[0][0] + v[1] * rotation.r[1][0] + v[2] * rotation.r[2][0]) +
            position[0],
        rotation.r[1][3] *
                (v[0] * rotation.r[0][1] + v[1] * rotation.r[1][1] + v[2] * rotation.r[2][1]) +
            position[1],
        rotation.r[2][3] *
                (v[0] * rotation.r[0][2] + v[1] * rotation.r[1][2] + v[2] * rotation.r[2][2]) +
            position[2]);
}

inline float3 Composed_transformation::object_to_world_vector(float3 const& v) const noexcept {
    return Vector3f_a(
        rotation.r[0][3] *
            (v[0] * rotation.r[0][0] + v[1] * rotation.r[1][0] + v[2] * rotation.r[2][0]),
        rotation.r[1][3] *
            (v[0] * rotation.r[0][1] + v[1] * rotation.r[1][1] + v[2] * rotation.r[2][1]),
        rotation.r[2][3] *
            (v[0] * rotation.r[0][2] + v[1] * rotation.r[1][2] + v[2] * rotation.r[2][2]));
}

}  // namespace scene::entity

#endif
