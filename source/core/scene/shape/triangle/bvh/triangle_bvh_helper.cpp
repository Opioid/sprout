#include "triangle_bvh_helper.hpp"
#include "base/math/plane.inl"
#include "base/math/vector3.inl"

namespace scene::shape::triangle::bvh {

float3 triangle_min(float3 const& a, float3 const& b, float3 const& c) {
    return min(a, min(b, c));
}

float3 triangle_max(float3 const& a, float3 const& b, float3 const& c) {
    return max(a, max(b, c));
}

Simd3f triangle_min(Simd3f const& a, Simd3f const& b, Simd3f const& c) {
    return min(a, min(b, c));
}

Simd3f triangle_max(Simd3f const& a, Simd3f const& b, Simd3f const& c) {
    return max(a, max(b, c));
}

float3 triangle_min(float3 const& a, float3 const& b, float3 const& c, float3 const& x) {
    return min(a, min(b, min(c, x)));
}

float3 triangle_max(float3 const& a, float3 const& b, float3 const& c, float3 const& x) {
    return max(a, max(b, max(c, x)));
}

float triangle_area(float3 const& a, float3 const& b, float3 const& c) {
    return 0.5f * length(cross(b - a, c - a));
}

uint32_t triangle_side(float3 const& a, float3 const& b, float3 const& c, math::Plane const& p) {
    uint32_t behind = 0;

    if (math::plane::behind(p, a)) {
        ++behind;
    }

    if (math::plane::behind(p, b)) {
        ++behind;
    }

    if (math::plane::behind(p, c)) {
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

bool triangle_completely_behind(float3 const& a, float3 const& b, float3 const& c,
                                math::Plane const& p) {
    if (math::plane::behind(p, a) && math::plane::behind(p, b) && math::plane::behind(p, c)) {
        return true;
    }

    return false;
}

}  // namespace scene::shape::triangle::bvh
