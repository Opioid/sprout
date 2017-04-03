#include "triangle_bvh_helper.hpp"
#include "base/math/vector3.inl"
#include "base/math/vector.inl"
#include "base/math/plane.inl"

namespace scene { namespace shape { namespace triangle { namespace bvh {

float3 triangle_min(float3_p a, float3_p b, float3_p c) {
	return math::min(a, math::min(b, c));
}

float3 triangle_max(float3_p a, float3_p b, float3_p c) {
	return math::max(a, math::max(b, c));
}

Vector triangle_min(FVector a,
								FVector b,
								FVector c) {
	return math::min(a, math::min(b, c));
}

Vector triangle_max(FVector a,
								FVector b,
								FVector c) {
	return math::max(a, math::max(b, c));
}

float3 triangle_min(float3_p a, float3_p b, float3_p c, float3_p x) {
	return math::min(a, math::min(b, math::min(c, x)));
}

float3 triangle_max(float3_p a, float3_p b, float3_p c, float3_p x) {
	return math::max(a, math::max(b, math::max(c, x)));
}

float triangle_area(float3_p a, float3_p b, float3_p c) {
	return 0.5f * math::length(math::cross(b - a, c - a));
}

uint32_t triangle_side(float3_p a, float3_p b, float3_p c, const math::Plane& p) {
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

bool triangle_completely_behind(float3_p a, float3_p b, float3_p c, const math::Plane& p) {
	if (math::plane::behind(p, a) && math::plane::behind(p, b) && math::plane::behind(p, c)) {
		return true;
	}

	return false;
}

}}}}
