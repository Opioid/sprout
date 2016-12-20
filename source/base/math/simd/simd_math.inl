#pragma once

#include "simd_math.hpp"

namespace math { namespace simd {

inline float rsqrt(float x) {
	Vector three = _mm_set1_ps(3.f);
	Vector half  = _mm_set1_ps(0.5f);
	Vector xs	 = _mm_load_ss(&x);
	Vector res   = _mm_rsqrt_ss(xs);
	Vector muls  = _mm_mul_ss(_mm_mul_ss(xs, res), res);
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

inline Vector rcp1(FVector x) {
	Vector rcp  = _mm_rcp_ss(x);
	Vector muls = _mm_mul_ss(_mm_mul_ss(rcp, rcp), x);
	return _mm_sub_ss(_mm_add_ss(rcp, rcp), muls);
}

}}
