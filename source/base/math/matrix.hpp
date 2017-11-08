#ifndef SU_BASE_MATH_MATRIX_HPP
#define SU_BASE_MATH_MATRIX_HPP

#include "simd/simd.hpp"

struct alignas(16) Matrix3 {
	Vector r[3];
};

struct alignas(16) Matrix4 {
	Vector r[4];
};

// Fix-up for (1st) XMMATRIX parameter to pass in-register on vector call; by reference otherwise
using VMatrix3 = const Matrix3;
using VMatrix4 = const Matrix4;

using RMatrix3 = const Matrix3&;
using RMatrix4 = const Matrix4&;

#endif
