#include "triangle_bvh_helper.hpp"
#include "base/math/plane.inl"
#include "base/math/simd_vector.inl"
#include "base/math/vector3.inl"

namespace scene::shape::triangle::bvh {

float3 triangle_min(float3 const& a, float3 const& b, float3 const& c) {
    return math::min(a, math::min(b, c));
}

float3 triangle_max(float3 const& a, float3 const& b, float3 const& c) {
    return math::max(a, math::max(b, c));
}

Vector triangle_min(FVector a, FVector b, FVector c) {
    return math::min(a, math::min(b, c));
}

Vector triangle_max(FVector a, FVector b, FVector c) {
    return math::max(a, math::max(b, c));
}

float3 triangle_min(float3 const& a, float3 const& b, float3 const& c, float3 const& x) {
    return math::min(a, math::min(b, math::min(c, x)));
}

float3 triangle_max(float3 const& a, float3 const& b, float3 const& c, float3 const& x) {
    return math::max(a, math::max(b, math::max(c, x)));
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
    } else if (0 == behind) {
        return 1;
    } else {
        return 2;
    }
}

bool triangle_completely_behind(float3 const& a, float3 const& b, float3 const& c,
                                math::Plane const& p) {
    if (math::plane::behind(p, a) && math::plane::behind(p, b) && math::plane::behind(p, c)) {
        return true;
    }

    return false;
}

}  // namespace scene::shape::triangle::bvh
