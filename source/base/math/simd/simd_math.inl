#pragma once

#include "simd_math.hpp"

namespace math { namespace simd {

inline Vector rsqrt(FVector x) {
	Vector three = _mm_set1_ps(3.f);
	Vector half	 = _mm_set1_ps(0.5f);
	Vector res	 = _mm_rsqrt_ps(x);
	Vector muls	 = _mm_mul_ps(_mm_mul_ps(x, res), res);
	return _mm_mul_ps(_mm_mul_ps(half, res), _mm_sub_ps(three, muls));
}

}}
