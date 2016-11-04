#include "triangle_bvh_helper.hpp"
#include "base/math/vector.inl"
#include "base/math/plane.inl"

namespace scene { namespace shape { namespace triangle { namespace bvh {

float3 triangle_min(const float3& a, const float3& b, const float3& c) {
	return math::min(a, math::min(b, c));
}

float3 triangle_max(const float3& a, const float3& b, const float3& c) {
	return math::max(a, math::max(b, c));
}

float3 triangle_min(const float3& a, const float3& b, const float3& c, const float3& x) {
	return math::min(a, math::min(b, math::min(c, x)));
}

float3 triangle_max(const float3& a, const float3& b, const float3& c, const float3& x) {
	return math::max(a, math::max(b, math::max(c, x)));
}

float triangle_area(const float3& a, const float3& b, const float3& c) {
	return 0.5f * math::length(math::cross(b - a, c - a));
}

uint32_t triangle_side(const float3& a, const float3& b, const float3& c, const math::plane& p) {
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

bool triangle_completely_behind(const float3& a, const float3& b, const float3& c,
								const math::plane& p) {
	if (math::behind(p, a) && math::behind(p, b) && math::behind(p, c)) {
		return true;
	}

	return false;
}

}}}}
