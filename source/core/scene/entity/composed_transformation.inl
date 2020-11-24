#ifndef SU_CORE_SCENE_ENTITY_COMPOSED_TRANSFORMATION_INL
#define SU_CORE_SCENE_ENTITY_COMPOSED_TRANSFORMATION_INL

#include "base/math/matrix4x4.inl"
#include "base/math/quaternion.inl"
#include "base/math/transformation.inl"
#include "composed_transformation.hpp"

namespace scene::entity {

inline void Composed_transformation::set(math::Transformation const& t) {
    float3x3 const rot = quaternion::create_matrix3x3(t.rotation);

    float4x4 const otw = compose(rot, t.scale, t.position);

    world_to_object = affine_inverted(otw);

    rotation = rot;

    rotation.r[0][3] = t.scale[0];
    rotation.r[1][3] = t.scale[1];
    rotation.r[2][3] = t.scale[2];

    position = t.position;
}

inline float Composed_transformation::scale_x() const {
    return rotation.r[0][3];
}

inline float Composed_transformation::scale_y() const {
    return rotation.r[1][3];
}

inline float Composed_transformation::scale_z() const {
    return rotation.r[2][3];
}

inline float2 Composed_transformation::scale_xy() const {
    return float2(rotation.r[0][3], rotation.r[1][3]);
}

inline float3 Composed_transformation::scale() const {
    return float3(rotation.r[0][3], rotation.r[1][3], rotation.r[2][3]);
}

inline float4x4 Composed_transformation::object_to_world() const {
    float3 const s(rotation.r[0][3], rotation.r[1][3], rotation.r[2][3]);
    return compose(rotation, s, position);
}

inline float3 Composed_transformation::world_to_object_point(float3_p p) const {
    return transform_point(world_to_object, p);
}

inline float3 Composed_transformation::world_to_object_vector(float3_p v) const {
    return transform_vector(world_to_object, v);
}

inline float3 Composed_transformation::world_to_object_normal(float3_p v) const {
    return transform_vector_transposed(rotation, v);
}

inline float3 Composed_transformation::object_to_world_point(float3_p v) const {
    float3 const s(rotation.r[0][3], rotation.r[1][3], rotation.r[2][3]);

    float3 const a = s[0] * rotation.r[0];
    float3 const b = s[1] * rotation.r[1];
    float3 const c = s[2] * rotation.r[2];

    return Vector3f_a((v[0] * a[0] + v[1] * b[0] + v[2] * c[0]) + position[0],
                      (v[0] * a[1] + v[1] * b[1] + v[2] * c[1]) + position[1],
                      (v[0] * a[2] + v[1] * b[2] + v[2] * c[2]) + position[2]);
}

inline float3 Composed_transformation::object_to_world_vector(float3_p v) const {
    float3 const s(rotation.r[0][3], rotation.r[1][3], rotation.r[2][3]);

    float3 const a = s[0] * rotation.r[0];
    float3 const b = s[1] * rotation.r[1];
    float3 const c = s[2] * rotation.r[2];

    return Vector3f_a((v[0] * a[0] + v[1] * b[0] + v[2] * c[0]),
                      (v[0] * a[1] + v[1] * b[1] + v[2] * c[1]),
                      (v[0] * a[2] + v[1] * b[2] + v[2] * c[2]));
}

inline float3 Composed_transformation::object_to_world_normal(float3_p v) const {
    return transform_vector(rotation, v);
}

}  // namespace scene::entity

#endif
