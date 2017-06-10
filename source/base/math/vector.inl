#pragma once

#include "vector.hpp"
#include "simd/simd_const.hpp"

namespace math {

/****************************************************************************
 *
 * Scalar operations
 *
 ****************************************************************************/

static inline Vector SU_CALLCONV add1(VVector a, VVector b) {
	return _mm_add_ss(a, b);
}

static inline Vector SU_CALLCONV mul1(VVector a, VVector b) {
	return _mm_mul_ss(a, b);
}

static inline Vector SU_CALLCONV min1(VVector a, VVector b) {
	return _mm_min_ss(a, b);
}

static inline Vector SU_CALLCONV max1(VVector a, VVector b) {
	return _mm_max_ss(a, b);
}

static inline Vector SU_CALLCONV sqrt1(VVector x) {
	Vector res   = _mm_rsqrt_ss(x);
	Vector muls  = _mm_mul_ss(_mm_mul_ss(x, res), res);
	return _mm_mul_ss(x, _mm_mul_ss(_mm_mul_ss(simd::Half, res), _mm_sub_ss(simd::Three, muls)));
}

static inline Vector SU_CALLCONV rcp1(VVector x) {
	Vector rcp  = _mm_rcp_ss(x);
	Vector muls = _mm_mul_ss(_mm_mul_ss(rcp, rcp), x);
	return _mm_sub_ss(_mm_add_ss(rcp, rcp), muls);
}

/****************************************************************************
 *
 * Vector operations
 *
 ****************************************************************************/

static inline Vector SU_CALLCONV add(VVector a, VVector b) {
	return _mm_add_ps(a, b);
}

static inline float SU_CALLCONV horizontal_sum(VVector a) {
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

static inline Vector SU_CALLCONV sub(VVector a, VVector b) {
	return _mm_sub_ps(a, b);
}

static inline Vector SU_CALLCONV mul(VVector a, VVector b) {
	return _mm_mul_ps(a, b);
}

static inline Vector SU_CALLCONV div(VVector a, VVector b) {
	return _mm_div_ps(a, b);
}

static inline Vector SU_CALLCONV dot3(VVector a, VVector b) {
	Vector mul  = _mm_mul_ps(a, b);
	Vector shuf = _mm_movehdup_ps(mul);
	Vector sums = _mm_add_ss(mul, shuf);
	shuf        = _mm_movehl_ps(shuf, sums);
	Vector dot  = _mm_add_ss(sums, shuf);
	// Splat x
	return SU_PERMUTE_PS(dot, _MM_SHUFFLE(0, 0, 0, 0));
}

static inline Vector SU_CALLCONV dot3_1(VVector a, VVector b) {
	Vector mul  = _mm_mul_ps(a, b);
	Vector shuf = _mm_movehdup_ps(mul);
	Vector sums = _mm_add_ss(mul, shuf);
	shuf        = _mm_movehl_ps(shuf, sums);
	return _mm_add_ss(sums, shuf);
}

static inline Vector sqrt(VVector x) {
	Vector res   = _mm_rsqrt_ps(x);
	Vector muls  = _mm_mul_ps(_mm_mul_ps(x, res), res);
	return _mm_mul_ps(x, _mm_mul_ps(_mm_mul_ps(simd::Half, res), _mm_sub_ps(simd::Three, muls)));
}

static inline Vector rsqrt(VVector x) {
	Vector res	 = _mm_rsqrt_ps(x);
	Vector muls	 = _mm_mul_ps(_mm_mul_ps(x, res), res);
	return _mm_mul_ps(_mm_mul_ps(simd::Half, res), _mm_sub_ps(simd::Three, muls));
}

static inline Vector normalized3(VVector v) {
	return mul(rsqrt(dot3(v, v)), v);
}

static inline Vector SU_CALLCONV cross3(VVector a, VVector b) {
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

static inline Vector SU_CALLCONV rcp(VVector x) {
	Vector rcp  = _mm_rcp_ps(x);
	Vector muls = _mm_mul_ps(_mm_mul_ps(rcp, rcp), x);
	return _mm_sub_ps(_mm_add_ps(rcp, rcp), muls);
}

static inline Vector SU_CALLCONV reciprocal3(VVector v) {
	Vector rcp = _mm_rcp_ps(v);
	rcp = _mm_and_ps(rcp, simd::Mask3);
	Vector mul = _mm_mul_ps(v, _mm_mul_ps(rcp, rcp));

	return _mm_sub_ps(_mm_add_ps(rcp, rcp), mul);
}

static inline Vector SU_CALLCONV min(VVector a, VVector b) {
	return _mm_min_ps(a, b);
}

static inline Vector SU_CALLCONV max(VVector a, VVector b) {
	return _mm_max_ps(a, b);
}

static inline Vector SU_CALLCONV splat_x(VVector v) {
	return SU_PERMUTE_PS(v, _MM_SHUFFLE(0, 0, 0, 0));
}

static inline void SU_CALLCONV sign(VVector v, uint32_t s[4]) {
	Vector sm = _mm_cmplt_ps(v, simd::Zero);
	__m128i smi = _mm_castps_si128(sm);
	smi = _mm_and_si128(smi, simd::Bool_mask);
	_mm_store_si128(reinterpret_cast<__m128i*>(s), smi);
}

}
