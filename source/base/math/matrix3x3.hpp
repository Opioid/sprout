#pragma once

#include "vector3.hpp"
#include "vector4.hpp"

namespace math {

template<typename T> struct Matrix4x4;

/****************************************************************************
 *
 * Generic 3x3 matrix
 *
 ****************************************************************************/

template<typename T> 
struct Matrix3x3 {
	union {
		struct {
			T m00, m01, m02,
			  m10, m11, m12, 
			  m20, m21, m22;
		};

		struct {
			T m[9]; 
		};

		struct {
			Vector3<T> rows[3];
		};

		struct {
			Vector3<T> x, y, z;
		};

		struct {
			Vector3<T> x, y, z;
		} v3;
	};

	Matrix3x3();

	Matrix3x3(T m00, T m01, T m02,
			  T m10, T m11, T m12,
			  T m20, T m21, T m22);
		
	Matrix3x3(const Vector3<T>& x, const Vector3<T>& y, const Vector3<T>& z);
	
	explicit Matrix3x3(const Matrix4x4<T>& m);

	Matrix3x3 operator*(const Matrix3x3& m) const;

	Matrix3x3 operator/(T s) const;

	Matrix3x3& operator*=(const Matrix3x3& m);

	static Matrix3x3 identity();
};

template<typename T>
Vector3<T> operator*(const Vector3<T>& v, const Matrix3x3<T>& m);

template<typename T>
Vector3<T>& operator*=(Vector3<T>& v, const Matrix3x3<T>& m);

template<typename T>
Vector3<T> transform_vector(const Matrix3x3<T>& m, const Vector3<T>& v);

template<typename T>
Vector3<T> transform_vector_transposed(const Matrix3x3<T>& m, const Vector3<T>& v);

template<typename T>
void transform_vectors(const Matrix3x3<T>& m,
					   const Vector3<T>& a, const Vector3<T>& b, Vector3<T>& oa, Vector3<T>& ob);

template<typename T>
Matrix3x3<T> normalized(const Matrix3x3<T>& m);

template<typename T>
T det(const Matrix3x3<T>& m);

template<typename T>
Matrix3x3<T> invert(const Matrix3x3<T>& m);

template<typename T>
void set_look_at(Matrix3x3<T>& m, const Vector3<T>& dir, const Vector3<T>& up);

// template<typename T>
// void set_basis(Matrix3x3<T>& m, const Vector3<T>& v);

template<typename T>
void set_scale(Matrix3x3<T>& m, T x, T y, T z);

template<typename T>
void set_scale(Matrix3x3<T>& m, const Vector3<T>& v);

template<typename T>
void scale(Matrix3x3<T>& m, const Vector3<T>& v);

template<typename T>
void set_rotation_x(Matrix3x3<T>& m, T a);

template<typename T>
void set_rotation_y(Matrix3x3<T>& m, T a);

template<typename T>
void set_rotation_z(Matrix3x3<T>& m, T a);

template<typename T>
void set_rotation(Matrix3x3<T>& m, const Vector3<T>& v, T a);

template<typename T>
Matrix3x3<T> transposed(const Matrix3x3<T>& m);

/****************************************************************************
 *
 * Aligned 3x3 float matrix
 *
 ****************************************************************************/

struct Vector4f_a;

struct alignas(16) Matrix3x3f_a {
	Vector3f_a r[3];

	Matrix3x3f_a() = default;

	constexpr Matrix3x3f_a(float m00, float m01, float m02,
						   float m10, float m11, float m12,
						   float m20, float m21, float m22) :
		r{Vector3f_a(m00, m01, m02),
		  Vector3f_a(m10, m11, m12),
		  Vector3f_a(m20, m21, m22)} {}

	static constexpr Matrix3x3f_a identity() {
		return Matrix3x3f_a(1.f, 0.f, 0.f,
							0.f, 1.f, 0.f,
							0.f, 0.f, 1.f);
	}
};

}

// using float3x3 = Matrix3x3<float>;
using float3x3 = math::Matrix3x3f_a;
