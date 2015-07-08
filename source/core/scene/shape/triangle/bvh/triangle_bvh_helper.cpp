#include "triangle_bvh_helper.hpp"
#include "base/math/vector.inl"
#include "base/math/plane.inl"

namespace scene { namespace shape { namespace triangle { namespace bvh {

math::float3 triangle_min(const math::float3& a, const math::float3& b, const math::float3& c, const math::float3& x) {
	return math::min(a, math::min(b, math::min(c, x)));
}

math::float3 triangle_max(const math::float3& a, const math::float3& b, const math::float3& c, const math::float3& x) {
	return math::max(a, math::max(b, math::max(c, x)));
}

float triangle_area(const math::float3& a, const math::float3& b, const math::float3& c) {
	return 0.5f * math::length(math::cross(b - a, c - a));
}

uint32_t triangle_side(const math::float3& a, const math::float3& b, const math::float3& c, const math::plane& p) {
	uint32_t behind = 0;

	if (math::behind(p, a)) {
		++behind;
	}

	if (math::behind(p, b)) {
		++behind;
	}

	if (math::behind(p, c)) {
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

}}}}
