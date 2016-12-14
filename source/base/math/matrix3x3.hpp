#pragma once

#include "vector3.hpp"

namespace math {

template<typename T> struct Vector3;
template<typename T> struct Matrix4x4;
template<typename T> struct Quaternion;
struct Vector4f_a;

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

	explicit Matrix3x3(const Quaternion<T>& q);

	Matrix3x3 operator*(const Matrix3x3& m) const;

	Matrix3x3 operator/(T s) const;

	Matrix3x3& operator*=(const Matrix3x3& m);

	static Matrix3x3 identity();
};

template<typename T>
Vector3<T> operator*(const Vector3<T>& v, const Matrix3x3<T>& m);

Vector3f_a operator*(FVector3f_a v, const Matrix3x3<float>& m);

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
	union {
		struct {
			float m00, m01, m02, pad0,
				  m10, m11, m12, pad1,
				  m20, m21, m22, pad2;
		};

		struct {
			float m[12];
		};

		struct {
			Vector3f_a x, y, z;
		} v;

		struct {
			Vector3f_a x, y, z;
		} v3;
	};

	Matrix3x3f_a();

	Matrix3x3f_a(float m00, float m01, float m02,
				 float m10, float m11, float m12,
				 float m20, float m21, float m22);

	Matrix3x3f_a(const Vector3f_a& x, const Vector3f_a& y, const Vector3f_a& z);

	explicit Matrix3x3f_a(const Vector4f_a& q);

	Matrix3x3f_a operator*(const Matrix3x3f_a& a) const;

	Matrix3x3f_a& operator*=(const Matrix3x3f_a& a);

	static Matrix3x3f_a identity();
};

Matrix3x3f_a create_matrix3x3(const Vector4f_a& q);

Vector3f_a operator*(FVector3f_a v, const Matrix3x3f_a& m);

Vector3f_a transform_vector(FVector3f_a v, const Matrix3x3f_a& m);

Vector3f_a transform_vector_transposed(FVector3f_a v, const Matrix3x3f_a& m);

void set_rotation_x(Matrix3x3f_a& m, float a);

void set_rotation_y(Matrix3x3f_a& m, float a);

void set_rotation_z(Matrix3x3f_a& m, float a);

void set_rotation(Matrix3x3f_a& m, const Vector3f_a& v, float a);

}
