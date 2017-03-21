#pragma once

#include "matrix.hpp"
#include "vector.inl"
#include "math/matrix4x4.inl"

namespace math {

/****************************************************************************
 *
 * Load operations
 *
 ****************************************************************************/

inline Matrix SU_CALLCONV load_float3x3(const Matrix3x3f_a& source) {
	Matrix m;
	m.r[0] = _mm_load_ps(&source.x.x);
	m.r[1] = _mm_load_ps(&source.y.x);
	m.r[2] = _mm_load_ps(&source.z.x);
	return m;
}

inline Matrix SU_CALLCONV load_float4x4(const Matrix4x4f_a& source) {
	Matrix m;
	m.r[0] = _mm_load_ps(&source.v.x.x);
	m.r[1] = _mm_load_ps(&source.v.y.x);
	m.r[2] = _mm_load_ps(&source.v.z.x);
	m.r[3] = _mm_load_ps(&source.v.w.x);
	return m;
}

/****************************************************************************
 *
 * 4x4 matrix operations
 *
 ****************************************************************************/

inline Vector SU_CALLCONV transform_vector(FMatrix m, HVector v) {
	Vector result = SU_PERMUTE_PS(v, _MM_SHUFFLE(0, 0, 0, 0));
	result = _mm_mul_ps(result, m.r[0]);
	Vector temp = SU_PERMUTE_PS(v, _MM_SHUFFLE(1, 1, 1, 1));
	temp = _mm_mul_ps(temp, m.r[1]);
	result = _mm_add_ps(result, temp);
	temp = SU_PERMUTE_PS(v, _MM_SHUFFLE(2, 2, 2, 2));
	temp = _mm_mul_ps(temp, m.r[2]);
	result = _mm_add_ps(result, temp);
	return result;
}

}
