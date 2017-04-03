#pragma once

#include "vector.hpp"
#include "simd.inl"
#include "vector4.inl"

namespace math {

/****************************************************************************
 *
 * Scalar operations
 *
 ****************************************************************************/

static inline Vector SU_CALLCONV add1(FVector a, FVector b) {
	return _mm_add_ss(a, b);
}

static inline Vector SU_CALLCONV mul1(FVector a, FVector b) {
	return _mm_mul_ss(a, b);
}

static inline Vector SU_CALLCONV min1(FVector a, FVector b) {
	return _mm_min_ss(a, b);
}

static inline Vector SU_CALLCONV max1(FVector a, FVector b) {
	return _mm_max_ss(a, b);
}

static inline Vector SU_CALLCONV rcp1(FVector x) {
	Vector rcp  = _mm_rcp_ss(x);
	Vector muls = _mm_mul_ss(_mm_mul_ss(rcp, rcp), x);
	return _mm_sub_ss(_mm_add_ss(rcp, rcp), muls);
}

/****************************************************************************
 *
 * Vector operations
 *
 ****************************************************************************/

static inline Vector SU_CALLCONV add(FVector a, FVector b) {
	return _mm_add_ps(a, b);
}

static inline float SU_CALLCONV horizontal_sum(FVector a) {
//	Vector t = _mm_hadd_ps(a, a);
//	t = _mm_hadd_ps(t, t);
//	float r;
//	_mm_store_ss(&r, t);
//	return r;

	Vector shuf = _mm_movehdup_ps(a);
	Vector sums = _mm_add_ps(a, shuf);
	shuf        = _mm_movehl_ps(shuf, sums);
	sums        = _mm_add_ss(sums, shuf);
	return        _mm_cvtss_f32(sums);
}

static inline Vector SU_CALLCONV sub(FVector a, FVector b) {
	return _mm_sub_ps(a, b);
}

static inline Vector SU_CALLCONV mul(FVector a, FVector b) {
	return _mm_mul_ps(a, b);
}

static inline Vector SU_CALLCONV div(FVector a, FVector b) {
	return _mm_div_ps(a, b);
}

static inline Vector SU_CALLCONV dot3(FVector a, FVector b) {
	Vector mul  = _mm_mul_ps(a, b);
	Vector shuf = _mm_movehdup_ps(mul);
	Vector sums = _mm_add_ss(mul, shuf);
	shuf        = _mm_movehl_ps(shuf, sums);
	Vector dot  = _mm_add_ss(sums, shuf);
	// Splat x
	return SU_PERMUTE_PS(dot, _MM_SHUFFLE(0, 0, 0, 0));
}

static inline Vector SU_CALLCONV dot3_1(FVector a, FVector b) {
	Vector mul  = _mm_mul_ps(a, b);
	Vector shuf = _mm_movehdup_ps(mul);
	Vector sums = _mm_add_ss(mul, shuf);
	shuf        = _mm_movehl_ps(shuf, sums);
	return _mm_add_ss(sums, shuf);
}

static inline Vector rsqrt(FVector x) {
	Vector three = _mm_set1_ps(3.f);
	Vector half	 = _mm_set1_ps(0.5f);
	Vector res	 = _mm_rsqrt_ps(x);
	Vector muls	 = _mm_mul_ps(_mm_mul_ps(x, res), res);
	return _mm_mul_ps(_mm_mul_ps(half, res), _mm_sub_ps(three, muls));
}

static inline Vector normalized3(FVector v) {
	return mul(rsqrt(dot3(v, v)), v);
}

static inline Vector SU_CALLCONV cross3(FVector a, FVector b) {
	// [ a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x ]

	// y1,z1,x1,w1
	Vector temp0 = SU_PERMUTE_PS(a, _MM_SHUFFLE(3, 0, 2, 1));
	// z2,x2,y2,w2
	Vector temp1 = SU_PERMUTE_PS(b, _MM_SHUFFLE(3, 1, 0, 2));
	// Perform the left operation
	Vector result = _mm_mul_ps(temp0, temp1);
	// z1,x1,y1,w1
	temp0 = SU_PERMUTE_PS(temp0, _MM_SHUFFLE(3, 0, 2, 1));
	// y2,z2,x2,w2
	temp1 = SU_PERMUTE_PS(temp1, _MM_SHUFFLE(3, 1, 0, 2));
	// Perform the right operation
	temp0 = _mm_mul_ps(temp0, temp1);
	// Subract the right from left, and return answer
	result = _mm_sub_ps(result, temp0);
	// Set w to zero
//	return _mm_and_ps(result, Mask3);
	return result;
}

static inline Vector SU_CALLCONV rcp(FVector x) {
	Vector rcp  = _mm_rcp_ps(x);
	Vector muls = _mm_mul_ps(_mm_mul_ps(rcp, rcp), x);
	return _mm_sub_ps(_mm_add_ps(rcp, rcp), muls);
}

static inline Vector SU_CALLCONV min(FVector a, FVector b) {
	return _mm_min_ps(a, b);
}

static inline Vector SU_CALLCONV max(FVector a, FVector b) {
	return _mm_max_ps(a, b);
}

static inline Vector SU_CALLCONV splat_x(FVector v) {
	return SU_PERMUTE_PS(v, _MM_SHUFFLE(0, 0, 0, 0));
}

}
