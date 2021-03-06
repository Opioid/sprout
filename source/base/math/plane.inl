#ifndef SU_BASE_MATH_PLANE_INL
#define SU_BASE_MATH_PLANE_INL

#include "plane.hpp"
#include "vector3.inl"
#include "vector4.inl"

namespace math::plane {

static inline Plane create(Vector3f_a_p normal, float d) {
    return Plane(normal, d);
}

static inline Plane create(Vector3f_a_p normal, Vector3f_a_p point) {
    return Plane(normal[0], normal[1], normal[2], -dot(normal, point));
}

static inline Plane create(Vector3f_a_p v0, Vector3f_a_p v1, Vector3f_a_p v2) {
    Vector3f_a const n = normalize(cross(v2 - v1, v0 - v1));

    return create(n, v0);
}

static inline float dot(Plane_p p, Vector3f_a_p v) {
    return (p[0] * v[0] + p[1] * v[1]) + (p[2] * v[2] + p[3]);
}

static inline bool behind(Plane_p p, Vector3f_a_p point) {
    return dot(p, point) < 0.f;
}

static inline Plane normalize(Plane_p p) {
    float const t = rlength(p.xyz());

    return t * p;
}

static inline Vector3f_a intersection(Plane_p p0, Plane_p p1, Plane_p p2) {
    Vector3f_a const n1(p0[0], p0[1], p0[2]);

    float const d1 = p0[3];

    Vector3f_a const n2(p1[0], p1[1], p1[2]);

    float const d2 = p1[3];

    Vector3f_a const n3(p2[0], p2[1], p2[2]);

    float const d3 = p2[3];

    //    d1 ( N2 * N3 ) + d2 ( N3 * N1 ) + d3 ( N1 * N2 )
    // P = ------------------------------------------------
    //                    N1 . ( N2 * N3 )

    return -(d1 * cross(n2, n3) + d2 * cross(n3, n1) + d3 * cross(n1, n2)) /
           dot(n1, (cross(n2, n3)));
}

}  // namespace math::plane

#endif
