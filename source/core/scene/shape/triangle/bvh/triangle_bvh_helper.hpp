#pragma once

#include "base/math/vector.hpp"
#include "base/math/plane.hpp"

namespace scene { namespace shape { namespace triangle { namespace bvh {

float3 triangle_min(float3_p a, float3_p b, float3_p c);
float3 triangle_max(float3_p a, float3_p b, float3_p c);

float3 triangle_min(float3_p a, float3_p b, float3_p c, float3_p x);
float3 triangle_max(float3_p a, float3_p b, float3_p c, float3_p x);

float triangle_area(float3_p a, float3_p b, float3_p c);

uint32_t triangle_side(float3_p a, float3_p b, float3_p c, const math::plane& p);

bool triangle_completely_behind(float3_p a, float3_p b, float3_p c, const math::plane& p);

}}}}
