#pragma once

#include "simd_matrix.hpp"
#include "simd_vector.inl"
#include "math/matrix4x4.inl"

namespace math { namespace simd {

/****************************************************************************
 *
 * Load operations
 *
 ****************************************************************************/

inline Matrix SU_CALLCONV load_float4x4(const Matrix4x4f_a& source) {
#if defined(_SU_NO_INTRINSICS_)
	Matrix M;
	M.r[0].vector4_f32[0] = pSource->m[0][0];
	M.r[0].vector4_f32[1] = pSource->m[0][1];
	M.r[0].vector4_f32[2] = pSource->m[0][2];
	M.r[0].vector4_f32[3] = pSource->m[0][3];

	M.r[1].vector4_f32[0] = pSource->m[1][0];
	M.r[1].vector4_f32[1] = pSource->m[1][1];
	M.r[1].vector4_f32[2] = pSource->m[1][2];
	M.r[1].vector4_f32[3] = pSource->m[1][3];

	M.r[2].vector4_f32[0] = pSource->m[2][0];
	M.r[2].vector4_f32[1] = pSource->m[2][1];
	M.r[2].vector4_f32[2] = pSource->m[2][2];
	M.r[2].vector4_f32[3] = pSource->m[2][3];

	M.r[3].vector4_f32[0] = pSource->m[3][0];
	M.r[3].vector4_f32[1] = pSource->m[3][1];
	M.r[3].vector4_f32[2] = pSource->m[3][2];
	M.r[3].vector4_f32[3] = pSource->m[3][3];
	return M;
#elif defined(_SU_SSE_INTRINSICS_)
	Matrix m;
	m.r[0] = _mm_load_ps(&source.x.x);
	m.r[1] = _mm_load_ps(&source.y.x);
	m.r[2] = _mm_load_ps(&source.z.x);
	m.r[3] = _mm_load_ps(&source.w.x);
	return m;
#endif
}

/****************************************************************************
 *
 * 4x4 matrix operations
 *
 ****************************************************************************/

inline Vector SU_CALLCONV transform_vector(FMatrix m, FVector v) {
#if defined(_SU_NO_INTRINSICS_)
	XMVECTOR Z = XMVectorSplatZ(V);
	XMVECTOR Y = XMVectorSplatY(V);
	XMVECTOR X = XMVectorSplatX(V);

	XMVECTOR Result = XMVectorMultiply(Z, M.r[2]);
	Result = XMVectorMultiplyAdd(Y, M.r[1], Result);
	Result = XMVectorMultiplyAdd(X, M.r[0], Result);

	return Result;
#elif defined(_SU_SSE_INTRINSICS_)
	Vector result = SU_PERMUTE_PS(v, _MM_SHUFFLE(0, 0, 0, 0));
	result = _mm_mul_ps(result, m.r[0]);
	Vector temp = SU_PERMUTE_PS(v, _MM_SHUFFLE(1, 1, 1, 1));
	temp = _mm_mul_ps(temp, m.r[1]);
	result = _mm_add_ps(result, temp);
	temp = SU_PERMUTE_PS(v, _MM_SHUFFLE(2, 2, 2, 2));
	temp = _mm_mul_ps(temp, m.r[2]);
	result = _mm_add_ps(result, temp);
	return result;
#endif
 }

}}
