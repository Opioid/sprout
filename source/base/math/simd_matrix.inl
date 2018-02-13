#ifndef SU_BASE_MATH_SIMD_MATRIX_INL
#define SU_BASE_MATH_SIMD_MATRIX_INL

#include "simd_matrix.hpp"
#include "simd_vector.inl"
#include "matrix4x4.inl"

namespace math {

//==============================================================================
// Load operations
//==============================================================================

inline Matrix3 SU_CALLCONV load_float3x3(const Matrix3x3f_a& source) {
	return Matrix3 {{
		_mm_load_ps(source.r[0].v),
		_mm_load_ps(source.r[1].v),
		_mm_load_ps(source.r[2].v)
	}};
}

inline Matrix4 SU_CALLCONV load_float4x4(const Matrix4x4f_a& source) {
	return Matrix4 {{
		_mm_load_ps(source.r[0].v),
		_mm_load_ps(source.r[1].v),
		_mm_load_ps(source.r[2].v),
		_mm_load_ps(source.r[3].v)
	}};
}

//==============================================================================
// Matrix operations
//==============================================================================

inline Vector SU_CALLCONV transform_vector(VMatrix3 m, VVector v) {
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

inline Vector SU_CALLCONV transform_vector(VMatrix4 m, VVector v) {
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

inline Vector SU_CALLCONV transform_point(VMatrix4 m, VVector v) {
	Vector result = SU_PERMUTE_PS(v, _MM_SHUFFLE(0, 0, 0, 0));
	result = _mm_mul_ps(result, m.r[0]);
	Vector temp = SU_PERMUTE_PS(v, _MM_SHUFFLE(1, 1, 1, 1));
	temp = _mm_mul_ps(temp, m.r[1]);
	result = _mm_add_ps(result, temp);
	temp = SU_PERMUTE_PS(v, _MM_SHUFFLE(2, 2, 2, 2));
	temp = _mm_mul_ps(temp, m.r[2]);
	result = _mm_add_ps(result, temp);
	result = _mm_add_ps(result, m.r[3]);
	return result;
}

}

#endif
