#pragma once

#include "base/math/plane.hpp"
#include "base/math/vector3.hpp"
#include "base/simd/simd.hpp"

namespace scene::shape::triangle::bvh {

float3 triangle_min(float3 const& a, float3 const& b, float3 const& c);
float3 triangle_max(float3 const& a, float3 const& b, float3 const& c);

Simd3f triangle_min(Simd3f const& a, Simd3f const& b, Simd3f const& c);

Simd3f triangle_max(Simd3f const& a, Simd3f const& b, Simd3f const& c);

float3 triangle_min(float3 const& a, float3 const& b, float3 const& c, float3 const& x);
float3 triangle_max(float3 const& a, float3 const& b, float3 const& c, float3 const& x);

float triangle_area(float3 const& a, float3 const& b, float3 const& c);

uint32_t triangle_side(float3 const& a, float3 const& b, float3 const& c, math::Plane const& p);

bool triangle_completely_behind(float3 const& a, float3 const& b, float3 const& c,
                                math::Plane const& p);

}  // namespace scene::shape::triangle::bvh
