#pragma once

#include "base/math/vector3.hpp"
#include "base/math/plane.hpp"
#include "base/simd/simd.hpp"

namespace scene::shape::triangle::bvh {

float3 triangle_min(const float3& a, const float3& b, const float3& c);
float3 triangle_max(const float3& a, const float3& b, const float3& c);

Vector triangle_min(VVector a, VVector b, VVector c);

Vector triangle_max(VVector a, VVector b, VVector c);

float3 triangle_min(const float3& a, const float3& b, const float3& c, const float3& x);
float3 triangle_max(const float3& a, const float3& b, const float3& c, const float3& x);

float triangle_area(const float3& a, const float3& b, const float3& c);

uint32_t triangle_side(const float3& a, const float3& b, const float3& c, const math::Plane& p);

bool triangle_completely_behind(const float3& a, const float3& b, const float3& c,
								const math::Plane& p);

}
