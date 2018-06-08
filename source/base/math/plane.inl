#ifndef SU_BASE_MATH_PLANE_INL
#define SU_BASE_MATH_PLANE_INL

#include "plane.hpp"
#include "vector3.inl"
#include "vector4.inl"

namespace math {
namespace plane {

static inline Plane create(Vector3f_a const& normal, float d) noexcept {
    return Plane(normal, d);
}

static inline Plane create(Vector3f_a const& normal, Vector3f_a const& point) noexcept {
    return Plane(normal[0], normal[1], normal[2], -dot(normal, point));
}

static inline Plane create(Vector3f_a const& v0, Vector3f_a const& v1,
                           Vector3f_a const& v2) noexcept {
    const Vector3f_a n = normalize(cross(v2 - v1, v0 - v1));

    return create(n, v0);
}

static inline float dot(const Vector4f_a& p, Vector3f_a const& v) noexcept {
    return (p[0] * v[0] + p[1] * v[1]) + (p[2] * v[2] + p[3]);
}

static inline bool behind(const Vector4f_a& p, Vector3f_a const& point) noexcept {
    return dot(p, point) < 0.f;
}

}  // namespace plane
}  // namespace math

#endif
