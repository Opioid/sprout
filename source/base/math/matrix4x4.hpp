#pragma once

#include "vector3.hpp"
#include "vector4.hpp"

namespace math {

template<typename T> struct Matrix3x3;
struct Matrix3x3f_a;
template<typename T> struct Transformation;
struct Transformationf_a;

/****************************************************************************
 *
 * Generic 4x4 matrix
 *
 ****************************************************************************/

template<typename T>
struct Matrix4x4 {
	union {
		struct {
			T m00, m01, m02, m03,
			  m10, m11, m12, m13,
			  m20, m21, m22, m23,
			  m30, m31, m32, m33;
		};

		struct {
			T m[16];
		};

		struct {
			Vector4<T> x, y, z, w;
		};

		struct {
			Vector4<T> rows[4];
		};
	};

	Matrix4x4();

	Matrix4x4(T m00, T m01, T m02, T m03,
			  T m10, T m11, T m12, T m13,
			  T m20, T m21, T m22, T m23,
			  T m30, T m31, T m32, T m33);
		
	Matrix4x4(const T m[16]);

	explicit Matrix4x4(const Matrix3x3<T>& m);

	explicit Matrix4x4(const Transformation<T>& t);

	explicit Matrix4x4(const Transformationf_a& t);

	Matrix4x4 operator*(const Matrix4x4& o) const;

	Matrix4x4 operator/(T s) const;

	Matrix4x4& operator*=(const Matrix4x4& m);

	static const Matrix4x4 identity;
};

template<typename T>
Matrix4x4<T> operator*(T s, const Matrix4x4<T>& m);

template<typename T>
Vector3<T> operator*(const Vector3<T>& v, const Matrix4x4<T>& m);

template<typename T>
Vector3<T> &operator*=(Vector3<T>& v, const Matrix4x4<T>& m);

template<typename T>
Vector4<T> operator*(const Vector4<T>& v, const Matrix4x4<T>& m);

template<typename T>
Vector3<T> transform_vector(const Matrix4x4<T>& m, const Vector3<T>& v);

template<typename T>
Vector3<T> transform_point(const Matrix4x4<T>& m, const Vector3<T>& v);

template<typename T>
void get_basis(Matrix3x3<T>& basis, const Matrix4x4<T>& m);

template<typename T>
Matrix3x3<T> extract_unscaled_basis(const Matrix4x4<T>& m);

template<typename T>
void set_basis(Matrix4x4<T>& m, const Matrix3x3<T>& basis);

template<typename T>
void get_origin(Vector3<T> &origin, const Matrix4x4<T>& m);

template<typename T>
void set_origin(Matrix4x4<T>& m, const Vector3<T>& origin);

template<typename T>
Vector3<T> get_scale(const Matrix4x4<T>& m);

template<typename T>
void set_scale(Matrix4x4<T>& m, T x, T y, T z);

template<typename T>
void set_scale(Matrix4x4<T>& m, const Vector3<T>& v);

template<typename T>
void scale(Matrix4x4<T>& m, const Vector3<T>& v);

template<typename T>
void set_translation(Matrix4x4<T>& m, T x, T y, T z);

template<typename T>
void set_translation(Matrix4x4<T>& m, const Vector3<T>& v);

template<typename T>
void set_rotation_x(Matrix4x4<T>& m, T a);

template<typename T>
void set_rotation_y(Matrix4x4<T>& m, T a);

template<typename T>
void set_rotation_z(Matrix4x4<T>& m, T a);

template<typename T>
void set_rotation(Matrix4x4<T>& m, const Vector3<T>& v, T a);

template<typename T>
Matrix4x4<T> transposed(const Matrix4x4<T>& m);

template<typename T>
T det(const Matrix4x4<T>& m);

template<typename T>
Matrix4x4<T> inverted(const Matrix4x4<T>& m);

template<typename T>
Matrix4x4<T> affine_inverted(const Matrix4x4<T>& m);

template<typename T>
void set_basis_scale_origin(Matrix4x4<T>& m,
							const Matrix3x3<T>& basis,
							const Vector3<T>& scale,
							const Vector3<T>& origin);

/****************************************************************************
 *
 * Aligned 4x4 float matrix
 *
 ****************************************************************************/

struct alignas(16) Matrix4x4f_a {
	Vector4f_a r[4];

	Matrix4x4f_a();

	Matrix4x4f_a(float m00, float m01, float m02, float m03,
				 float m10, float m11, float m12, float m13,
				 float m20, float m21, float m22, float m23,
				 float m30, float m31, float m32, float m33);

	explicit Matrix4x4f_a(const Transformationf_a& t);

	Vector3f_a x() const;
	Vector3f_a y() const;
	Vector3f_a z() const;
	Vector3f_a w() const;

	Matrix4x4f_a operator*(const Matrix4x4f_a& o) const;
};

Vector3f_a transform_vector(FVector3f_a v, const Matrix4x4f_a& m);

Vector3f_a transform_vector_transposed(FVector3f_a v, const Matrix4x4f_a& m);

Vector3f_a transform_point(FVector3f_a v, const Matrix4x4f_a& m);

Matrix4x4f_a create_matrix4x4(FVector4f_a q);

Matrix4x4f_a affine_inverted(const Matrix4x4f_a& m);

void set_basis_scale_origin(Matrix4x4f_a& m,
							const Matrix3x3f_a& basis,
							const Vector3f_a& scale,
							const Vector3f_a& origin);

void set_basis_scale_origin(Matrix4x4f_a& m,
							const Matrix4x4f_a& basis,
							const Vector3f_a& scale,
							const Vector3f_a& origin);

void set_translation(Matrix4x4f_a& m, const Vector3f_a& v);

void set_rotation_x(Matrix4x4f_a& m, float a);

}
