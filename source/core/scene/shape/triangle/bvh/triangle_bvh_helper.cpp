#include "triangle_bvh_helper.hpp"
#include "base/math/plane.inl"
#include "base/math/vector3.inl"

namespace scene::shape::triangle::bvh {

float3 triangle_min(float3_p a, float3_p b, float3_p c) {
    return min(a, min(b, c));
}

float3 triangle_max(float3_p a, float3_p b, float3_p c) {
    return max(a, max(b, c));
}

Simdf triangle_min(Simdf_p a, Simdf_p b, Simdf_p c) {
    return min(a, min(b, c));
}

Simdf triangle_max(Simdf_p a, Simdf_p b, Simdf_p c) {
    return max(a, max(b, c));
}

float3 triangle_min(float3_p a, float3_p b, float3_p c, float3_p x) {
    return min(a, min(b, min(c, x)));
}

float3 triangle_max(float3_p a, float3_p b, float3_p c, float3_p x) {
    return max(a, max(b, max(c, x)));
}

float triangle_area(float3_p a, float3_p b, float3_p c) {
    return 0.5f * length(cross(b - a, c - a));
}

uint32_t triangle_side(float3_p a, float3_p b, float3_p c, Plane_p p) {
    uint32_t behind = 0;

    if (plane::behind(p, a)) {
        ++behind;
    }

    if (plane::behind(p, b)) {
        ++behind;
    }

    if (plane::behind(p, c)) {
        ++behind;
    }

    if (3 == behind) {
        return 0;
    }

    if (0 == behind) {
        return 1;
    }

    return 2;
}

bool triangle_completely_behind(float3_p a, float3_p b, float3_p c, Plane_p p) {
    return plane::behind(p, a) && plane::behind(p, b) && plane::behind(p, c);
}

}  // namespace scene::shape::triangle::bvh
