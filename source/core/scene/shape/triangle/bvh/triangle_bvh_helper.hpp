#pragma once

#include "base/math/vector3.hpp"
#include "base/math/plane.hpp"
#include "base/math/vector.hpp"

namespace scene { namespace shape { namespace triangle { namespace bvh {

float3 triangle_min(float3_p a, float3_p b, float3_p c);
float3 triangle_max(float3_p a, float3_p b, float3_p c);

Vector triangle_min(FVector a,
								FVector b,
								FVector c);

Vector triangle_max(FVector a,
								FVector b,
								FVector c);

float3 triangle_min(float3_p a, float3_p b, float3_p c, float3_p x);
float3 triangle_max(float3_p a, float3_p b, float3_p c, float3_p x);

float triangle_area(float3_p a, float3_p b, float3_p c);

uint32_t triangle_side(float3_p a, float3_p b, float3_p c, const math::Plane& p);

bool triangle_completely_behind(float3_p a, float3_p b, float3_p c, const math::Plane& p);

}}}}
