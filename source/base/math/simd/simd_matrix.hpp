#pragma once

#include "simd.hpp"
#include "simd_vector.hpp"

namespace math { namespace simd {

#if (defined(_M_IX86) || defined(_M_X64)) && defined(_SU_NO_INTRINSICS_)
	struct Matrix {
#else
	struct alignas(16) Matrix {
#endif
#ifdef _SU_NO_INTRINSICS_
	union {
		XMVECTOR r[4];
		struct
		{
			float _11, _12, _13, _14;
			float _21, _22, _23, _24;
			float _31, _32, _33, _34;
			float _41, _42, _43, _44;
		};
		float m[4][4];
	};
#else
	Vector r[4];
#endif
};

// Fix-up for (1st) XMMATRIX parameter to pass in-register on vector call; by reference otherwise
#if (_SU_VECTORCALL_ ) && !defined(_SU_NO_INTRINSICS_)
	typedef const Matrix FMatrix;
#else
	typedef const Matrix& FMatrix;
#endif

// Fix-up for (2nd+) XMMATRIX parameters to pass by reference
typedef const Matrix& CMatrix;

/****************************************************************************
 *
 * Load operations
 *
 ****************************************************************************/

Matrix SU_CALLCONV load_float4x4(const Matrix4x4f_a& source);

/****************************************************************************
 *
 * 4x4 matrix operations
 *
 ****************************************************************************/

 Vector SU_CALLCONV transform_vector(FMatrix m, FVector v);

}}
