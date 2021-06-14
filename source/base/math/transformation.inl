#ifndef SU_BASE_MATH_TRANSFORMATION_INL
#define SU_BASE_MATH_TRANSFORMATION_INL

#include "matrix4x4.inl"
#include "quaternion.inl"
#include "transformation.hpp"
#include "vector3.inl"

namespace math {

inline bool Transformation::operator==(Transformation const& o) const {
    return position == o.position && scale == o.scale && rotation == o.rotation;
}

inline bool Transformation::operator!=(Transformation const& o) const {
    return position != o.position || scale != o.scale || rotation != o.rotation;
}

inline void Transformation::set(Transformation const& other, float3_p camera_pos) {
    position = other.position - camera_pos;
    scale    = other.scale;
    rotation = other.rotation;
}

static inline Transformation transform(Transformation const& a, Transformation const& b) {
    return {transform_point(float4x4(b), a.position), a.scale,
            quaternion::mul(b.rotation, a.rotation)};
}

static inline Transformation lerp(Transformation const& a, Transformation const& b, float t) {
    return {lerp(a.position, b.position, t), lerp(a.scale, b.scale, t),
            quaternion::slerp(a.rotation, b.rotation, t)};
}

}  // namespace math

#endif
