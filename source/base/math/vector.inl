#pragma once

#include "vector.hpp"
#include "vector4.inl"

namespace math {


/****************************************************************************
 *
 * Load operations
 *
 ****************************************************************************/

static inline Vector SU_CALLCONV load_float(float x) {
	return _mm_set1_ps(x);
}

static inline Vector SU_CALLCONV load_float3(const Vector3<float>& source) {
	__m128 x = _mm_load_ss(&source.v[0]);
	__m128 y = _mm_load_ss(&source.v[1]);
	__m128 z = _mm_load_ss(&source.v[2]);
	__m128 xy = _mm_unpacklo_ps(x, y);
	return _mm_movelh_ps(xy, z);
}

static inline Vector SU_CALLCONV load_float3(const float* source) {
	// Reads an extra float which is zero'd
	__m128 v = _mm_load_ps(source);
	return _mm_and_ps(v, Mask3);
}

static inline Vector SU_CALLCONV load_float3(const Vector3f_a& source) {
	// Reads an extra float which is zero'd
	__m128 v = _mm_load_ps(source.v);
	return _mm_and_ps(v, Mask3);
}

static inline Vector SU_CALLCONV load_float4(const Vector3f_a& source) {
	// Reads an extra float!!!
	return _mm_load_ps(source.v);
}

static inline Vector SU_CALLCONV load_float4(const Vector4f_a& source) {
	return _mm_load_ps(source.v);
}


/****************************************************************************
 *
 * Store operations
 *
 ****************************************************************************/

static inline void SU_CALLCONV store_float3(Vector3<float>& destination, FVector v) {
	Vector t1 = SU_PERMUTE_PS(v, _MM_SHUFFLE(1, 1, 1, 1));
	Vector t2 = SU_PERMUTE_PS(v, _MM_SHUFFLE(2, 2, 2, 2));
	_mm_store_ss(&destination.v[0], v);
	_mm_store_ss(&destination.v[1], t1);
	_mm_store_ss(&destination.v[2], t2);
}

static inline void SU_CALLCONV store_float3(Vector3f_a& destination, FVector v) {
	Vector t = SU_PERMUTE_PS(v, _MM_SHUFFLE(2, 2, 2, 2));
	_mm_storel_epi64(reinterpret_cast<__m128i*>(&destination), _mm_castps_si128(v));
	_mm_store_ss(&destination[2], t);
}

static inline void SU_CALLCONV store_float3_unsafe(Vector3f_a& destination, FVector v) {
	_mm_store_ps(destination.v, v);
}

static inline void SU_CALLCONV store_float4(Vector4f_a& destination, FVector v) {
	_mm_store_ps(destination.v, v);
}

static inline void SU_CALLCONV store_float3_unsafe(float* destination, FVector v) {
	_mm_store_ps(destination, v);
}

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

/****************************************************************************
 *
 * 3D vector operations
 *
 ****************************************************************************/

static inline Vector SU_CALLCONV add3(FVector a, FVector b) {
	return _mm_add_ps(a, b);
}

static inline Vector SU_CALLCONV sub3(FVector a, FVector b) {
	return _mm_sub_ps(a, b);
}

static inline Vector SU_CALLCONV mul3(FVector a, FVector b) {
	return _mm_mul_ps(a, b);
}

static inline Vector SU_CALLCONV div3(FVector a, FVector b) {
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
	return mul3(rsqrt(dot3(v, v)), v);
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

static inline Vector SU_CALLCONV min3(FVector a, FVector b) {
	return _mm_min_ps(a, b);
}

static inline Vector SU_CALLCONV max3(FVector a, FVector b) {
	return _mm_max_ps(a, b);
}

static inline Vector SU_CALLCONV splat_x(FVector v) {
	return SU_PERMUTE_PS(v, _MM_SHUFFLE(0, 0, 0, 0));
}

}
