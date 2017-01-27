#pragma once

#include "base/math/vector.hpp"
#include "base/math/plane.hpp"
#include "base/math/simd/simd_vector.hpp"

namespace scene { namespace shape { namespace triangle { namespace bvh {

float3 triangle_min(float3_p a, float3_p b, float3_p c);
float3 triangle_max(float3_p a, float3_p b, float3_p c);

math::simd::Vector triangle_min(math::simd::FVector a,
								math::simd::FVector b,
								math::simd::FVector c);

math::simd::Vector triangle_max(math::simd::FVector a,
								math::simd::FVector b,
								math::simd::FVector c);

float3 triangle_min(float3_p a, float3_p b, float3_p c, float3_p x);
float3 triangle_max(float3_p a, float3_p b, float3_p c, float3_p x);

float triangle_area(float3_p a, float3_p b, float3_p c);

uint32_t triangle_side(float3_p a, float3_p b, float3_p c, const math::plane& p);

bool triangle_completely_behind(float3_p a, float3_p b, float3_p c, const math::plane& p);

}}}}
