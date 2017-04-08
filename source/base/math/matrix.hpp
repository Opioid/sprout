#pragma once

#include "simd/simd.hpp"

struct alignas(16) Matrix3 {
	Vector r[3];
};

struct alignas(16) Matrix4 {
	Vector r[4];
};

// Fix-up for (1st) XMMATRIX parameter to pass in-register on vector call; by reference otherwise
#if (_SU_VECTORCALL_ )
	using FMatrix3 = const Matrix3;
	using FMatrix4 = const Matrix4;
#else
	using FMatrix3 = const Matrix3&;
	using FMatrix4 = const Matrix4&;
#endif

// Fix-up for (2nd+) XMMATRIX parameters to pass by reference
using CMatrix3 = const Matrix3&;
using CMatrix4 = const Matrix4&;
