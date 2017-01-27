#include "triangle_bvh_helper.hpp"
#include "base/math/vector.inl"
#include "base/math/plane.inl"
#include "base/math/simd/simd_vector.inl"

namespace scene { namespace shape { namespace triangle { namespace bvh {

float3 triangle_min(float3_p a, float3_p b, float3_p c) {
	return math::min(a, math::min(b, c));
}

float3 triangle_max(float3_p a, float3_p b, float3_p c) {
	return math::max(a, math::max(b, c));
}

math::simd::Vector triangle_min(math::simd::FVector a,
								math::simd::FVector b,
								math::simd::FVector c) {
	return math::simd::min3(a, math::simd::min3(b, c));
}

math::simd::Vector triangle_max(math::simd::FVector a,
								math::simd::FVector b,
								math::simd::FVector c) {
	return math::simd::max3(a, math::simd::max3(b, c));
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

uint32_t triangle_side(float3_p a, float3_p b, float3_p c, const math::plane& p) {
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

bool triangle_completely_behind(float3_p a, float3_p b, float3_p c, const math::plane& p) {
	if (math::behind(p, a) && math::behind(p, b) && math::behind(p, c)) {
		return true;
	}

	return false;
}

}}}}
