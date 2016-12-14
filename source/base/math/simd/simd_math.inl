#pragma once

#include "simd_math.hpp"

namespace math { namespace simd {

inline float rsqrt(float x) {
	__m128 three = _mm_set1_ps(3.f);
	__m128 half  = _mm_set1_ps(0.5f);
	__m128 xs	 = _mm_load_ss(&x);
	__m128 res   = _mm_rsqrt_ss(xs);
	__m128 muls  = _mm_mul_ss(_mm_mul_ss(xs, res), res);
	_mm_store_ss(&x, _mm_mul_ss(_mm_mul_ss(half, res), _mm_sub_ss(three, muls)));
	return x;
}

inline Vector rsqrt(FVector x) {
	Vector three = _mm_set1_ps(3.f);
	Vector half	 = _mm_set1_ps(0.5f);
	Vector res	 = _mm_rsqrt_ps(x);
	Vector muls	 = _mm_mul_ps(_mm_mul_ps(x, res), res);
	return _mm_mul_ps(_mm_mul_ps(half, res), _mm_sub_ps(three, muls));
}

}}
