#pragma once

// This is basically a subset of DirectXMath!

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
	// Perform the dot product
	Vector vDot = _mm_mul_ps(a, b);
	// x=Dot.vector4_f32[1], y=Dot.vector4_f32[2]
	Vector vTemp = SU_PERMUTE_PS(vDot, _MM_SHUFFLE(2, 1, 2, 1));
	// Result.vector4_f32[0] = x+y
	vDot = _mm_add_ss(vDot, vTemp);
	// x=Dot.vector4_f32[2]
	vTemp = SU_PERMUTE_PS(vTemp, _MM_SHUFFLE(1, 1, 1, 1));
	// Result.vector4_f32[0] = (x+y)+z
	vDot = _mm_add_ss(vDot, vTemp);
	// Splat x
	return SU_PERMUTE_PS(vDot, _MM_SHUFFLE(0, 0, 0, 0));
}

static inline Vector SU_CALLCONV dot3_1(FVector a, FVector b) {
	// Perform the dot product
	Vector vDot = _mm_mul_ps(a, b);
	// x=Dot.vector4_f32[1], y=Dot.vector4_f32[2]
	Vector vTemp = SU_PERMUTE_PS(vDot, _MM_SHUFFLE(2, 1, 2, 1));
	// Result.vector4_f32[0] = x+y
	vDot = _mm_add_ss(vDot, vTemp);
	// x=Dot.vector4_f32[2]
	vTemp = SU_PERMUTE_PS(vTemp, _MM_SHUFFLE(1, 1, 1, 1));
	// Result.vector4_f32[0] = (x+y)+z
	return _mm_add_ss(vDot, vTemp);
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
