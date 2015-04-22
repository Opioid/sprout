#pragma once

#include "base/math/vector.hpp"
#include "base/math/plane.hpp"

namespace scene { namespace shape { namespace triangle { namespace bvh {

math::float3 triangle_min(const math::float3& a, const math::float3& b, const math::float3& c, const math::float3& x);
math::float3 triangle_max(const math::float3& a, const math::float3& b, const math::float3& c, const math::float3& x);

uint32_t triangle_side(const math::float3& a, const math::float3& b, const math::float3& c, const math::plane& p);

}}}}
