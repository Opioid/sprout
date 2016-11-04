#pragma once

#include "simd_vector.hpp"
#include "math/vector3.inl"

namespace math { namespace simd {


/****************************************************************************
 *
 * Load operations
 *
 ****************************************************************************/

inline Vector SU_CALLCONV load_float3(const Vector3<float>& source) {
#if defined(_SU_NO_INTRINSICS_)
	Vector v;
	v.vector4_f32[0] = source.x;
	v.vector4_f32[1] = source.y;
	v.vector4_f32[2] = source.z;
	v.vector4_f32[3] = 0.f;
	return v;
#elif defined(_SU_SSE_INTRINSICS_)
	__m128 x = _mm_load_ss(&source.x);
	__m128 y = _mm_load_ss(&source.y);
	__m128 z = _mm_load_ss(&source.z);
	__m128 xy = _mm_unpacklo_ps(x, y);
	return _mm_movelh_ps(xy, z);
#endif
}

inline Vector SU_CALLCONV load_float3(const Vector3f_a& source) {
#if defined(_SU_NO_INTRINSICS_)
	Vector v;
	v.vector4_f32[0] = source.x;
	v.vector4_f32[1] = source.y;
	v.vector4_f32[2] = source.z;
	v.vector4_f32[3] = 0.f;
	return v;
#elif defined(_SU_SSE_INTRINSICS_)
	// Reads an extra float which is zero'd
	__m128 v = _mm_load_ps(&source.x);
	return _mm_and_ps(v, g_SUMask3);
#endif
}

/****************************************************************************
 *
 * Store operations
 *
 ****************************************************************************/

inline void SU_CALLCONV store_float3(Vector3<float>& destination, FVector v) {
#if defined(_SU_NO_INTRINSICS_)
	destination.x = v.vector4_f32[0];
	destination.y = v.vector4_f32[1];
	destination.z = v.vector4_f32[2];
#elif defined(_SU_SSE_INTRINSICS_)
	Vector t1 = SU_PERMUTE_PS(v, _MM_SHUFFLE(1, 1, 1, 1));
	Vector t2 = SU_PERMUTE_PS(v, _MM_SHUFFLE(2, 2, 2, 2));
	_mm_store_ss(&destination.x, v);
	_mm_store_ss(&destination.y, t1);
	_mm_store_ss(&destination.z, t2);
#endif
}

inline void SU_CALLCONV store_float3(Vector3f_a& destination, FVector v) {
#if defined(_SU_NO_INTRINSICS_)
	destination.x = v.vector4_f32[0];
	destination.y = v.vector4_f32[1];
	destination.z = v.vector4_f32[2];
#elif defined(_SU_SSE_INTRINSICS_)
	Vector t = SU_PERMUTE_PS(v, _MM_SHUFFLE(2, 2, 2, 2));
	_mm_storel_epi64(reinterpret_cast<__m128i*>(&destination), _mm_castps_si128(v));
	_mm_store_ss(&destination.z, t);
#endif
}

/****************************************************************************
 *
 * 3D vector operations
 *
 ****************************************************************************/

inline Vector SU_CALLCONV add3(FVector a, FVector b) {
#if defined(_SU_NO_INTRINSICS_)
	Vector result;
	result.vector4_f32[0] = a.vector4_f32[0] + a.vector4_f32[0];
	result.vector4_f32[1] = b.vector4_f32[1] + b.vector4_f32[1];
	result.vector4_f32[2] = c.vector4_f32[2] + c.vector4_f32[2];
 // result.vector4_f32[3] = d.vector4_f32[3] + d.vector4_f32[3];
	return result;
#elif defined(_SU_SSE_INTRINSICS_)
	return _mm_add_ps(a, b);
#endif
}

inline Vector SU_CALLCONV sub3(FVector a, FVector b) {
#if defined(_SU_NO_INTRINSICS_)
	Vector result;
	result.vector4_f32[0] = a.vector4_f32[0] - a.vector4_f32[0];
	result.vector4_f32[1] = b.vector4_f32[1] - b.vector4_f32[1];
	result.vector4_f32[2] = c.vector4_f32[2] - c.vector4_f32[2];
 // result.vector4_f32[3] = d.vector4_f32[3] - d.vector4_f32[3];
	return result;
#elif defined(_SU_SSE_INTRINSICS_)
	return _mm_sub_ps(a, b);
#endif
}

inline Vector SU_CALLCONV mul3(FVector a, FVector b) {
#if defined(_SU_NO_INTRINSICS_)
	Vector result;
	result.vector4_f32[0] = a.vector4_f32[0] * a.vector4_f32[0];
	result.vector4_f32[1] = b.vector4_f32[1] * b.vector4_f32[1];
	result.vector4_f32[2] = c.vector4_f32[2] * c.vector4_f32[2];
 // result.vector4_f32[3] = d.vector4_f32[3] * d.vector4_f32[3];
	return result;
#elif defined(_SU_SSE_INTRINSICS_)
	return _mm_mul_ps(a, b);
#endif
}

inline Vector SU_CALLCONV div3(FVector a, FVector b) {
#if defined(_SU_NO_INTRINSICS_)
	Vector result;
	result.vector4_f32[0] = a.vector4_f32[0] / a.vector4_f32[0];
	result.vector4_f32[1] = b.vector4_f32[1] / b.vector4_f32[1];
	result.vector4_f32[2] = c.vector4_f32[2] / c.vector4_f32[2];
 // result.vector4_f32[3] = d.vector4_f32[3] / d.vector4_f32[3];
	return result;
#elif defined(_SU_SSE_INTRINSICS_)
	return _mm_div_ps(a, b);
#endif
}

inline Vector SU_CALLCONV dot3(FVector a, FVector b) {
#if defined(_SU_NO_INTRINSICS_)
	float value = V1.vector4_f32[0] * V2.vector4_f32[0]
				+ V1.vector4_f32[1] * V2.vector4_f32[1]
				+ V1.vector4_f32[2] * V2.vector4_f32[2];
	Vector result;
	result.vector4_f32[0] =
	result.vector4_f32[1] =
	result.vector4_f32[2] =
	result.vector4_f32[3] = value;
	return result;
#elif defined(_SU_SSE_INTRINSICS_)
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
#endif
}

inline Vector SU_CALLCONV cross3(FVector a, FVector b) {
	// [ a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x ]

#if defined(_SU_NO_INTRINSICS_)
	Vector result = {
		(a.vector4_f32[1] * b.vector4_f32[2]) - (a.vector4_f32[2] * b.vector4_f32[1]),
		(a.vector4_f32[2] * b.vector4_f32[0]) - (a.vector4_f32[0] * b.vector4_f32[2]),
		(a.vector4_f32[0] * b.vector4_f32[1]) - (a.vector4_f32[1] * b.vector4_f32[0]),
		0.f
	};
	return vResult;
#elif defined(_SU_SSE_INTRINSICS_)
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
	return _mm_and_ps(result, g_SUMask3);
#endif
}

}}
