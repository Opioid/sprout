#ifndef SU_BASE_MATH_PLANE_INL
#define SU_BASE_MATH_PLANE_INL

#include "plane.hpp"
#include "vector3.inl"
#include "vector4.inl"

namespace math::plane {

static inline Plane create(Vector3f_a const& normal, float d) noexcept {
    return Plane(normal, d);
}

static inline Plane create(Vector3f_a const& normal, Vector3f_a const& point) noexcept {
    return Plane(normal[0], normal[1], normal[2], -dot(normal, point));
}

static inline Plane create(Vector3f_a const& v0, Vector3f_a const& v1,
                           Vector3f_a const& v2) noexcept {
    Vector3f_a const n = normalize(cross(v2 - v1, v0 - v1));

    return create(n, v0);
}

static inline float dot(Plane const& p, Vector3f_a const& v) noexcept {
    return (p[0] * v[0] + p[1] * v[1]) + (p[2] * v[2] + p[3]);
}

static inline bool behind(Plane const& p, Vector3f_a const& point) noexcept {
    return dot(p, point) < 0.f;
}

static inline Plane normalize(Plane const& p) noexcept {
    float const t = 1.f / length(p.xyz());

    return t * p;
}

}  // namespace math::plane

#endif
