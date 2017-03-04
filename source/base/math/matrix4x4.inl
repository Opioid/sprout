#pragma once

#include "matrix4x4.hpp"
#include "transformation.inl"

namespace math {

/****************************************************************************
 *
 * Generic 4x4 matrix
 *
 ****************************************************************************/

template<typename T>
Matrix4x4<T>::Matrix4x4() {}

template<typename T>
Matrix4x4<T>::Matrix4x4(T m00, T m01, T m02, T m03,
						T m10, T m11, T m12, T m13,
						T m20, T m21, T m22, T m23,
						T m30, T m31, T m32, T m33) :
	m00(m00), m01(m01), m02(m02), m03(m03),
	m10(m10), m11(m11), m12(m12), m13(m13),
	m20(m20), m21(m21), m22(m22), m23(m23),
	m30(m30), m31(m31), m32(m32), m33(m33) {}

template<typename T>
Matrix4x4<T>::Matrix4x4(const T m[16]) {
	m00 = m[0];  m01 = m[1];  m02 = m[2];  m03 = m[3];
	m10 = m[4];  m11 = m[5];  m12 = m[6];  m13 = m[7];
	m20 = m[8];  m21 = m[9];  m22 = m[10]; m23 = m[11];
	m30 = m[12]; m31 = m[13]; m32 = m[14]; m33 = m[15];
}

template<typename T>
Matrix4x4<T>::Matrix4x4(const Matrix3x3<T>& m) :
	m00(m.m00), m01(m.m01), m02(m.m02), m03(T(0)),
	m10(m.m10), m11(m.m11), m12(m.m12), m13(T(0)),
	m20(m.m20), m21(m.m21), m22(m.m22), m23(T(0)),
	m30(T(0)),  m31(T(0)),  m32(T(0)),  m33(T(1)) {}

template<typename T>
Matrix4x4<T>::Matrix4x4(const Transformation<T>& t) {
	set_basis_scale_origin(*this, float3x3(t.rotation), t.scale, t.position);
}

template<typename T>
Matrix4x4<T>::Matrix4x4(const Transformationf_a& t) {
	set_basis_scale_origin(*this, create_matrix3x3(t.rotation), t.scale, t.position);
}

template<typename T>
Matrix4x4<T> Matrix4x4<T>::operator*(const Matrix4x4& o) const {
	return Matrix4x4(m00 * o.m00 + m01 * o.m10 + m02 * o.m20 + m03 * o.m30,
					 m00 * o.m01 + m01 * o.m11 + m02 * o.m21 + m03 * o.m31,
					 m00 * o.m02 + m01 * o.m12 + m02 * o.m22 + m03 * o.m32,
					 m00 * o.m03 + m01 * o.m13 + m02 * o.m23 + m03 * o.m33,

					 m10 * o.m00 + m11 * o.m10 + m12 * o.m20 + m13 * o.m30,
					 m10 * o.m01 + m11 * o.m11 + m12 * o.m21 + m13 * o.m31,
					 m10 * o.m02 + m11 * o.m12 + m12 * o.m22 + m13 * o.m32,
					 m10 * o.m03 + m11 * o.m13 + m12 * o.m23 + m13 * o.m33,

					 m20 * o.m00 + m21 * o.m10 + m22 * o.m20 + m23 * o.m30,
					 m20 * o.m01 + m21 * o.m11 + m22 * o.m21 + m23 * o.m31,
					 m20 * o.m02 + m21 * o.m12 + m22 * o.m22 + m23 * o.m32,
					 m20 * o.m03 + m21 * o.m13 + m22 * o.m23 + m23 * o.m33,

					 m30 * o.m00 + m31 * o.m10 + m32 * o.m20 + m33 * o.m30,
					 m30 * o.m01 + m31 * o.m11 + m32 * o.m21 + m33 * o.m31,
					 m30 * o.m02 + m31 * o.m12 + m32 * o.m22 + m33 * o.m32,
					 m30 * o.m03 + m31 * o.m13 + m32 * o.m23 + m33 * o.m33);
}

template<typename T>
Matrix4x4<T> Matrix4x4<T>::operator/(T s) const {
	T is = T(1) / s;
	return Matrix4x4(is * m00, is * m01, is * m02, is * m03,
					 is * m10, is * m11, is * m12, is * m13,
					 is * m20, is * m21, is * m22, is * m23,
					 is * m30, is * m31, is * m32, is * m33);
}

template<typename T>
Matrix4x4<T>& Matrix4x4<T>::operator*=(const Matrix4x4& m) {
	Matrix4x4 temp(m00 * m.m00 + m01 * m.m10 + m02 * m.m20 + m03 * m.m30,
				   m00 * m.m01 + m01 * m.m11 + m02 * m.m21 + m03 * m.m31,
				   m00 * m.m02 + m01 * m.m12 + m02 * m.m22 + m03 * m.m32,
				   m00 * m.m03 + m01 * m.m13 + m02 * m.m23 + m03 * m.m33,

				   m10 * m.m00 + m11 * m.m10 + m12 * m.m20 + m13 * m.m30,
				   m10 * m.m01 + m11 * m.m11 + m12 * m.m21 + m13 * m.m31,
				   m10 * m.m02 + m11 * m.m12 + m12 * m.m22 + m13 * m.m32,
				   m10 * m.m03 + m11 * m.m13 + m12 * m.m23 + m13 * m.m33,

				   m20 * m.m00 + m21 * m.m10 + m22 * m.m20 + m23 * m.m30,
				   m20 * m.m01 + m21 * m.m11 + m22 * m.m21 + m23 * m.m31,
				   m20 * m.m02 + m21 * m.m12 + m22 * m.m22 + m23 * m.m32,
				   m20 * m.m03 + m21 * m.m13 + m22 * m.m23 + m23 * m.m33,

				   m30 * m.m00 + m31 * m.m10 + m32 * m.m20 + m33 * m.m30,
				   m30 * m.m01 + m31 * m.m11 + m32 * m.m21 + m33 * m.m31,
				   m30 * m.m02 + m31 * m.m12 + m32 * m.m22 + m33 * m.m32,
				   m30 * m.m03 + m31 * m.m13 + m32 * m.m23 + m33 * m.m33);

	return *this = temp;
}

template<typename T>
const Matrix4x4<T> Matrix4x4<T>::identity = Matrix4x4<T>(T(1), T(0), T(0), T(0),
														 T(0), T(1), T(0), T(0),
														 T(0), T(0), T(1), T(0),
														 T(0), T(0), T(0), T(1));

template<typename T>
Matrix4x4<T> operator*(T s, const Matrix4x4<T>& m) {
	return Matrix4x4<T>(s * m.m00, s * m.m01, s * m.m02, s * m.m03,
						s * m.m10, s * m.m11, s * m.m12, s * m.m13,
						s * m.m20, s * m.m21, s * m.m22, s * m.m23,
						s * m.m30, s * m.m31, s * m.m32, s * m.m33);
}

template<typename T>
Vector3<T> operator*(const Vector3<T>& v, const Matrix4x4<T>& m) {
	return Vector3<T>(v.x * m.m00 + v.y * m.m10 + v.z * m.m20 + m.m30,
					  v.x * m.m01 + v.y * m.m11 + v.z * m.m21 + m.m31,
					  v.x * m.m02 + v.y * m.m12 + v.z * m.m22 + m.m32);
}

template<typename T>
Vector3<T> &operator*=(Vector3<T>& v, const Matrix4x4<T>& m) {
	Vector3<T> temp(v.x * m.m00 + v.y * m.m10 + v.z * m.m20 + m.m30,
					v.x * m.m01 + v.y * m.m11 + v.z * m.m21 + m.m31,
					v.x * m.m02 + v.y * m.m12 + v.z * m.m22 + m.m32);

	return v = temp;
}

template<typename T>
Vector4<T> operator*(const Vector4<T>& v, const Matrix4x4<T>& m) {
	return Vector4<T>(v.x * m.m00 + v.y * m.m10 + v.z * m.m20 + v.w * m.m30,
					  v.x * m.m01 + v.y * m.m11 + v.z * m.m21 + v.w * m.m31,
					  v.x * m.m02 + v.y * m.m12 + v.z * m.m22 + v.w * m.m32,
					  v.x * m.m03 + v.y * m.m13 + v.z * m.m23 + v.w * m.m33);
}

template<typename T>
Vector3<T> transform_vector(const Matrix4x4<T>& m, const Vector3<T>& v) {
	return Vector3<T>(v.x * m.m00 + v.y * m.m10 + v.z * m.m20,
					  v.x * m.m01 + v.y * m.m11 + v.z * m.m21,
					  v.x * m.m02 + v.y * m.m12 + v.z * m.m22);
}

template<typename T>
Vector3<T> transform_point(const Matrix4x4<T>& m, const Vector3<T>& v) {
	return Vector3<T>(v.x * m.m00 + v.y * m.m10 + v.z * m.m20 + m.m30,
					  v.x * m.m01 + v.y * m.m11 + v.z * m.m21 + m.m31,
					  v.x * m.m02 + v.y * m.m12 + v.z * m.m22 + m.m32);
}

template<typename T>
void get_basis(Matrix3x3<T>& basis, const Matrix4x4<T>& m) {
	basis.m00 = m.m00; basis.m01 = m.m01; basis.m02 = m.m02;
	basis.m10 = m.m10; basis.m11 = m.m11; basis.m12 = m.m12;
	basis.m20 = m.m20; basis.m21 = m.m21; basis.m22 = m.m22;
}

template<typename T>
Matrix3x3<T> extract_unscaled_basis(const Matrix4x4<T>& m) {
	return Matrix3x3<T>(normalize(m.rows[0].xyz),
						normalize(m.rows[1].xyz),
						normalize(m.rows[2].xyz));
}

template<typename T>
void set_basis(Matrix4x4<T>& m, const Matrix3x3<T>& basis) {
	m.m00 = basis.m00; m.m01 = basis.m01; m.m02 = basis.m02;
	m.m10 = basis.m10; m.m11 = basis.m11; m.m12 = basis.m12;
	m.m20 = basis.m20; m.m21 = basis.m21; m.m22 = basis.m22;
}

template<typename T>
void get_origin(Vector3<T> &origin, const Matrix4x4<T>& m)
{
	origin.x = m.m30; origin.y = m.m31; origin.z = m.m32;
}

template<typename T>
void set_origin(Matrix4x4<T>& m, const Vector3<T>& origin) {
	m.m30 = origin.x; m.m31 = origin.y; m.m32 = origin.z;
}

template<typename T>
Vector3<T> get_scale(const Matrix4x4<T>& m) {
	return Vector3<T>(length(m.rows[0].xyz), length(m.rows[1].xyz), length(m.rows[2].xyz));
}

template<typename T>
void set_scale(Matrix4x4<T>& m, T x, T y, T z) {
	m.m00 = x;    m.m01 = T(0); m.m02 = T(0); m.m03 = T(0);
	m.m10 = T(0); m.m11 = y;    m.m12 = T(0); m.m13 = T(0);
	m.m20 = T(0); m.m21 = T(0); m.m22 = z;    m.m23 = T(0);
	m.m30 = T(0); m.m31 = T(0); m.m32 = T(0); m.m33 = T(1);
}

template<typename T>
void set_scale(Matrix4x4<T>& m, const Vector3<T>& v) {
	m.m00 = v.x;  m.m01 = T(0); m.m02 = T(0); m.m03 = T(0);
	m.m10 = T(0); m.m11 = v.y;  m.m12 = T(0); m.m13 = T(0);
	m.m20 = T(0); m.m21 = T(0); m.m22 = v.z;  m.m23 = T(0);
	m.m30 = T(0); m.m31 = T(0); m.m32 = T(0); m.m33 = T(1);
}

template<typename T>
void scale(Matrix4x4<T>& m, const Vector3<T>& v) {
	m.m00 *= v.x; m.m01 *= v.x; m.m02 *= v.x;
	m.m10 *= v.y; m.m11 *= v.y; m.m12 *= v.y;
	m.m20 *= v.z; m.m21 *= v.z; m.m22 *= v.z;
}

template<typename T>
void set_translation(Matrix4x4<T>& m, T x, T y, T z) {
	m.m00 = T(1); m.m01 = T(0); m.m02 = T(0); m.m03 = T(0);
	m.m10 = T(0); m.m11 = T(1); m.m12 = T(0); m.m13 = T(0);
	m.m20 = T(0); m.m21 = T(0); m.m22 = T(1); m.m23 = T(0);
	m.m30 = x;    m.m31 = y;    m.m32 = z;    m.m33 = T(1);
}

template<typename T>
void set_translation(Matrix4x4<T>& m, const Vector3<T>& v) {
	m.m00 = T(1); m.m01 = T(0); m.m02 = T(0); m.m03 = T(0);
	m.m10 = T(0); m.m11 = T(1); m.m12 = T(0); m.m13 = T(0);
	m.m20 = T(0); m.m21 = T(0); m.m22 = T(1); m.m23 = T(0);
	m.m30 = v.x;  m.m31 = v.y;  m.m32 = v.z;  m.m33 = T(1);
}

template<typename T>
void set_rotation_x(Matrix4x4<T>& m, T a) {
	T c = std::cos(a);
	T s = std::sin(a);

	m.m00 = T(1); m.m01 = T(0); m.m02 =  T(0); m.m03 = T(0);
	m.m10 = T(0); m.m11 = c;    m.m12 = -s;    m.m13 = T(0);
	m.m20 = T(0); m.m21 = s;    m.m22 =  c;    m.m23 = T(0);
	m.m30 = T(0); m.m31 = T(0); m.m32 =  T(0); m.m33 = T(1);
}

template<typename T>
void set_rotation_y(Matrix4x4<T>& m, T a) {
	T c = std::cos(a);
	T s = std::sin(a);

	m.m00 =  c;    m.m01 = T(0); m.m02 = s;    m.m03 = T(0);
	m.m10 =  T(0); m.m11 = T(1); m.m12 = T(0); m.m13 = T(0);
	m.m20 = -s;    m.m21 = T(0); m.m22 = c;    m.m23 = T(0);
	m.m30 =  T(0); m.m31 = T(0); m.m32 = T(0); m.m33 = T(1);
}

template<typename T>
void set_rotation_z(Matrix4x4<T>& m, T a) {
	T c = std::cos(a);
	T s = std::sin(a);

	m.m00 = c;    m.m01 = -s;   m.m02 = T(0); m.m03 = T(0);
	m.m10 = s;    m.m11 = c;    m.m12 = T(0); m.m13 = T(0);
	m.m20 = T(0); m.m21 = T(0); m.m22 = T(1); m.m23 = T(0);
	m.m30 = T(0); m.m31 = T(0); m.m32 = T(0); m.m33 = T(1);
}

template<typename T>
void set_rotation(Matrix4x4<T>& m, const Vector3<T>& v, T a) {
	T c = std::cos(a);
	T s = std::sin(a);
	T t = T(1) - c;

	m.m00 = c + v.x * v.x * t;
	m.m11 = c + v.y * v.y * t;
	m.m22 = c + v.z * v.z * t;

	T tmp1 = v.x * v.y * t;
	T tmp2 = v.z * s;

	m.m10 = tmp1 + tmp2;
	m.m01 = tmp1 - tmp2;

	tmp1 = v.x * v.z * t;
	tmp2 = v.y * s;

	m.m20 = tmp1 - tmp2;
	m.m02 = tmp1 + tmp2;

	tmp1 = v.y * v.z * t;
	tmp2 = v.x * s;

	m.m21 = tmp1 + tmp2;
	m.m12 = tmp1 - tmp2;

	m.m03 = T(0); m.m13 = T(0); m.m23 = T(0);
	m.m30 = T(0); m.m31 = T(0); m.m32 = T(0); m.m33 = T(1);
}

template<typename T>
Matrix4x4<T> transposed(const Matrix4x4<T>& m) {
	return Matrix4x4<T>(m.m00, m.m10, m.m20, m.m30,
						m.m01, m.m11, m.m21, m.m31,
						m.m02, m.m12, m.m22, m.m32,
						m.m03, m.m13, m.m23, m.m33);
}

template<typename T>
T det(const Matrix4x4<T>& m) {
	return m.m00 * m.m11 * m.m22 * m.m33 + m.m00 * m.m12 * m.m23 * m.m31 + m.m00 * m.m13 * m.m21 * m.m32
		 + m.m01 * m.m10 * m.m23 * m.m32 + m.m01 * m.m12 * m.m20 * m.m33 + m.m01 * m.m13 * m.m22 * m.m30
		 + m.m02 * m.m10 * m.m21 * m.m33 + m.m02 * m.m11 * m.m23 * m.m30 + m.m02 * m.m13 * m.m20 * m.m31
		 + m.m03 * m.m10 * m.m22 * m.m31 + m.m03 * m.m11 * m.m21 * m.m32 + m.m03 * m.m12 * m.m21 * m.m30

		 - m.m00 * m.m11 * m.m23 * m.m32 - m.m00 * m.m12 * m.m21 * m.m33 - m.m00 * m.m13 * m.m22 * m.m31
		 - m.m01 * m.m10 * m.m22 * m.m33 - m.m01 * m.m12 * m.m23 * m.m30 - m.m01 * m.m13 * m.m20 * m.m32
		 - m.m02 * m.m10 * m.m23 * m.m31 - m.m02 * m.m11 * m.m20 * m.m33 - m.m02 * m.m13 * m.m21 * m.m30
		 - m.m03 * m.m10 * m.m21 * m.m32 - m.m03 * m.m11 * m.m22 * m.m30 - m.m03 * m.m12 * m.m20 * m.m31;
}

template<typename T>
Matrix4x4<T> inverted(const Matrix4x4<T>& m) {
	return Matrix4x4<T>(m.m11*m.m22*m.m33 + m.m12*m.m23*m.m31 + m.m13*m.m21*m.m32 - m.m11*m.m23*m.m32 - m.m12*m.m21*m.m33 - m.m13*m.m22*m.m31,
						m.m01*m.m23*m.m32 + m.m02*m.m21*m.m33 + m.m03*m.m22*m.m31 - m.m01*m.m22*m.m33 - m.m02*m.m23*m.m31 - m.m03*m.m21*m.m32,
						m.m01*m.m12*m.m33 + m.m02*m.m13*m.m31 + m.m03*m.m11*m.m32 - m.m01*m.m13*m.m32 - m.m02*m.m11*m.m33 - m.m03*m.m12*m.m31,
						m.m01*m.m13*m.m22 + m.m02*m.m11*m.m23 + m.m03*m.m12*m.m21 - m.m01*m.m12*m.m23 - m.m02*m.m13*m.m21 - m.m03*m.m11*m.m22,

						m.m10*m.m23*m.m32 + m.m12*m.m20*m.m33 + m.m13*m.m22*m.m30 - m.m10*m.m22*m.m33 - m.m12*m.m23*m.m30 - m.m13*m.m20*m.m32,
						m.m00*m.m22*m.m33 + m.m02*m.m23*m.m30 + m.m03*m.m20*m.m32 - m.m00*m.m23*m.m32 - m.m02*m.m20*m.m33 - m.m03*m.m22*m.m30,
						m.m00*m.m13*m.m32 + m.m02*m.m10*m.m33 + m.m03*m.m12*m.m30 - m.m00*m.m12*m.m33 - m.m02*m.m13*m.m30 - m.m03*m.m10*m.m32,
						m.m00*m.m12*m.m23 + m.m02*m.m13*m.m20 + m.m03*m.m10*m.m22 - m.m00*m.m13*m.m22 - m.m02*m.m10*m.m23 - m.m03*m.m12*m.m20,

						m.m10*m.m21*m.m33 + m.m11*m.m23*m.m30 + m.m13*m.m20*m.m31 - m.m10*m.m23*m.m31 - m.m11*m.m20*m.m33 - m.m13*m.m21*m.m30,
						m.m00*m.m23*m.m31 + m.m01*m.m20*m.m33 + m.m03*m.m21*m.m30 - m.m00*m.m21*m.m33 - m.m01*m.m23*m.m30 - m.m03*m.m20*m.m31,
						m.m00*m.m11*m.m33 + m.m01*m.m13*m.m30 + m.m03*m.m10*m.m31 - m.m00*m.m13*m.m31 - m.m01*m.m10*m.m33 - m.m03*m.m11*m.m30,
						m.m00*m.m13*m.m21 + m.m01*m.m10*m.m23 + m.m03*m.m11*m.m20 - m.m00*m.m11*m.m23 - m.m01*m.m13*m.m20 - m.m03*m.m10*m.m21,

						m.m10*m.m22*m.m31 + m.m11*m.m20*m.m32 + m.m12*m.m21*m.m30 - m.m10*m.m21*m.m32 - m.m11*m.m22*m.m30 - m.m12*m.m20*m.m31,
						m.m00*m.m21*m.m32 + m.m01*m.m22*m.m30 + m.m02*m.m20*m.m31 - m.m00*m.m22*m.m31 - m.m01*m.m20*m.m32 - m.m02*m.m21*m.m30,
						m.m00*m.m12*m.m31 + m.m01*m.m10*m.m32 + m.m02*m.m11*m.m30 - m.m00*m.m11*m.m32 - m.m01*m.m12*m.m30 - m.m02*m.m10*m.m31,
						m.m00*m.m11*m.m22 + m.m01*m.m12*m.m20 + m.m02*m.m10*m.m21 - m.m00*m.m12*m.m21 - m.m01*m.m10*m.m22 - m.m02*m.m11*m.m20) / det(m);
}

template<typename T>
Matrix4x4<T> affine_inverted(const Matrix4x4<T>& m) {
	float m00_11 = m.m00 * m.m11;
	float m01_12 = m.m01 * m.m12;
	float m02_10 = m.m02 * m.m10;
	float m00_12 = m.m00 * m.m12;
	float m01_10 = m.m01 * m.m10;
	float m02_11 = m.m02 * m.m11;

	float id = 1.f / (m00_11 * m.m22 + m01_12 * m.m20 + m02_10 * m.m21
					- m00_12 * m.m21 - m01_10 * m.m22 - m02_11 * m.m20);

	float m11_22 = m.m11 * m.m22;
	float m12_21 = m.m12 * m.m21;
	float m02_21 = m.m02 * m.m21;
	float m01_22 = m.m01 * m.m22;
	float m12_20 = m.m12 * m.m20;
	float m10_22 = m.m10 * m.m22;
	float m00_22 = m.m00 * m.m22;
	float m02_20 = m.m02 * m.m20;
	float m10_21 = m.m10 * m.m21;
	float m11_20 = m.m11 * m.m20;
	float m01_20 = m.m01 * m.m20;
	float m00_21 = m.m00 * m.m21;

	return Matrix4x4<T>(
		(m11_22 - m12_21) * id,
		(m02_21 - m01_22) * id,
		(m01_12 - m02_11) * id,
		0.f,

		(m12_20 - m10_22) * id,
		(m00_22 - m02_20) * id,
		(m02_10 - m00_12) * id,
		0.f,

		(m10_21 - m11_20) * id,
		(m01_20 - m00_21) * id,
		(m00_11 - m01_10) * id,
		0.f,

		(m10_22 * m.m31 + m11_20 * m.m32 + m12_21 * m.m30 - m10_21 * m.m32 - m11_22 * m.m30 - m12_20 * m.m31) * id,
		(m00_21 * m.m32 + m01_22 * m.m30 + m02_20 * m.m31 - m00_22 * m.m31 - m01_20 * m.m32 - m02_21 * m.m30) * id,
		(m00_12 * m.m31 + m01_10 * m.m32 + m02_11 * m.m30 - m00_11 * m.m32 - m01_12 * m.m30 - m02_10 * m.m31) * id,
		1.f);
}

template<typename T>
void set_basis_scale_origin(Matrix4x4<T>& m,
							const Matrix3x3<T>& basis,
							const Vector3<T>& scale,
							const Vector3<T>& origin) {
	m.m00 = basis.m00 * scale.x; m.m01 = basis.m01 * scale.x; m.m02 = basis.m02 * scale.x; m.m03 = T(0);
	m.m10 = basis.m10 * scale.y; m.m11 = basis.m11 * scale.y; m.m12 = basis.m12 * scale.y; m.m13 = T(0);
	m.m20 = basis.m20 * scale.z; m.m21 = basis.m21 * scale.z; m.m22 = basis.m22 * scale.z; m.m23 = T(0);
	m.m30 = origin.x;			 m.m31 = origin.y;			  m.m32 = origin.z;			   m.m33 = T(1);

}

/****************************************************************************
 *
 * Aligned 4x4 float matrix
 *
 ****************************************************************************/

inline Matrix4x4f_a::Matrix4x4f_a() {}

inline Matrix4x4f_a::Matrix4x4f_a(float m00, float m01, float m02, float m03,
								  float m10, float m11, float m12, float m13,
								  float m20, float m21, float m22, float m23,
								  float m30, float m31, float m32, float m33) :
	m00(m00), m01(m01), m02(m02), m03(m03),
	m10(m10), m11(m11), m12(m12), m13(m13),
	m20(m20), m21(m21), m22(m22), m23(m23),
	m30(m30), m31(m31), m32(m32), m33(m33) {}

inline Matrix4x4f_a::Matrix4x4f_a(const Transformationf_a& t) {
	set_basis_scale_origin(*this, math::create_matrix3x3(t.rotation), t.scale, t.position);
}

inline Matrix4x4f_a Matrix4x4f_a::operator*(const Matrix4x4f_a& o) const {
	return Matrix4x4f_a((m00 * o.m00 + m01 * o.m10) + (m02 * o.m20 + m03 * o.m30),
						(m00 * o.m01 + m01 * o.m11) + (m02 * o.m21 + m03 * o.m31),
						(m00 * o.m02 + m01 * o.m12) + (m02 * o.m22 + m03 * o.m32),
						(m00 * o.m03 + m01 * o.m13) + (m02 * o.m23 + m03 * o.m33),

						(m10 * o.m00 + m11 * o.m10) + (m12 * o.m20 + m13 * o.m30),
						(m10 * o.m01 + m11 * o.m11) + (m12 * o.m21 + m13 * o.m31),
						(m10 * o.m02 + m11 * o.m12) + (m12 * o.m22 + m13 * o.m32),
						(m10 * o.m03 + m11 * o.m13) + (m12 * o.m23 + m13 * o.m33),

						(m20 * o.m00 + m21 * o.m10) + (m22 * o.m20 + m23 * o.m30),
						(m20 * o.m01 + m21 * o.m11) + (m22 * o.m21 + m23 * o.m31),
						(m20 * o.m02 + m21 * o.m12) + (m22 * o.m22 + m23 * o.m32),
						(m20 * o.m03 + m21 * o.m13) + (m22 * o.m23 + m23 * o.m33),

						(m30 * o.m00 + m31 * o.m10) + (m32 * o.m20 + m33 * o.m30),
						(m30 * o.m01 + m31 * o.m11) + (m32 * o.m21 + m33 * o.m31),
						(m30 * o.m02 + m31 * o.m12) + (m32 * o.m22 + m33 * o.m32),
						(m30 * o.m03 + m31 * o.m13) + (m32 * o.m23 + m33 * o.m33));
}

inline Vector3f_a transform_vector(FVector3f_a v, const Matrix4x4f_a& m) {
	return Vector3f_a(v.x * m.m00 + v.y * m.m10 + v.z * m.m20,
					  v.x * m.m01 + v.y * m.m11 + v.z * m.m21,
					  v.x * m.m02 + v.y * m.m12 + v.z * m.m22);
}

inline Vector3f_a transform_vector_transposed(FVector3f_a v, const Matrix4x4f_a& m) {
	return Vector3f_a(v.x * m.m00 + v.y * m.m01 + v.z * m.m02,
					  v.x * m.m10 + v.y * m.m11 + v.z * m.m12,
					  v.x * m.m20 + v.y * m.m21 + v.z * m.m22);
}

inline Vector3f_a transform_point(FVector3f_a v, const Matrix4x4f_a& m) {
	return Vector3f_a((v.x * m.m00 + v.y * m.m10) + (v.z * m.m20 + m.m30),
					  (v.x * m.m01 + v.y * m.m11) + (v.z * m.m21 + m.m31),
					  (v.x * m.m02 + v.y * m.m12) + (v.z * m.m22 + m.m32));
}

inline Matrix4x4f_a create_matrix4x4(const Vector4f_a& q) {
	const float d = dot(q, q);

	const float s = 2.f / d;

	const float xs = q.v[0] * s,  ys = q.v[1] * s,  zs = q.v[2] * s;
	const float wx = q.v[3] * xs, wy = q.v[3] * ys, wz = q.v[3] * zs;
	const float xx = q.v[0] * xs, xy = q.v[0] * ys, xz = q.v[0] * zs;
	const float yy = q.v[1] * ys, yz = q.v[1] * zs, zz = q.v[2] * zs;

	return Matrix4x4f_a(1.f - (yy + zz), xy - wz,         xz + wy,			0.f,
						xy + wz,         1.f - (xx + zz), yz - wx,			0.f,
						xz - wy,         yz + wx,         1.f - (xx + yy),	0.f,
						0.f,			 0.f,			  0.f,				1.f);
}

inline Matrix4x4f_a affine_inverted(const Matrix4x4f_a& m) {
	const float m00_11 = m.m00 * m.m11;
	const float m01_12 = m.m01 * m.m12;
	const float m02_10 = m.m02 * m.m10;
	const float m00_12 = m.m00 * m.m12;
	const float m01_10 = m.m01 * m.m10;
	const float m02_11 = m.m02 * m.m11;

	const float id = 1.f / ((m00_11 * m.m22 + m01_12 * m.m20 + m02_10 * m.m21) -
							(m00_12 * m.m21 + m01_10 * m.m22 + m02_11 * m.m20));

	const float m11_22 = m.m11 * m.m22;
	const float m12_21 = m.m12 * m.m21;
	const float m02_21 = m.m02 * m.m21;
	const float m01_22 = m.m01 * m.m22;
	const float m12_20 = m.m12 * m.m20;
	const float m10_22 = m.m10 * m.m22;
	const float m00_22 = m.m00 * m.m22;
	const float m02_20 = m.m02 * m.m20;
	const float m10_21 = m.m10 * m.m21;
	const float m11_20 = m.m11 * m.m20;
	const float m01_20 = m.m01 * m.m20;
	const float m00_21 = m.m00 * m.m21;

	return Matrix4x4f_a((m11_22 - m12_21) * id,
						(m02_21 - m01_22) * id,
						(m01_12 - m02_11) * id,
						0.f,

						(m12_20 - m10_22) * id,
						(m00_22 - m02_20) * id,
						(m02_10 - m00_12) * id,
						0.f,

						(m10_21 - m11_20) * id,
						(m01_20 - m00_21) * id,
						(m00_11 - m01_10) * id,
						0.f,

						((m10_22 * m.m31 + m11_20 * m.m32 + m12_21 * m.m30) -
						 (m10_21 * m.m32 + m11_22 * m.m30 + m12_20 * m.m31)) * id,
						((m00_21 * m.m32 + m01_22 * m.m30 + m02_20 * m.m31) -
						 (m00_22 * m.m31 + m01_20 * m.m32 + m02_21 * m.m30)) * id,
						((m00_12 * m.m31 + m01_10 * m.m32 + m02_11 * m.m30) -
						 (m00_11 * m.m32 + m01_12 * m.m30 + m02_10 * m.m31)) * id,
						1.f);
}

inline void set_basis_scale_origin(Matrix4x4f_a& m,
								   const Matrix3x3f_a& basis,
								   const Vector3f_a& scale,
								   const Vector3f_a& origin) {
	m.m00 = basis.m00 * scale.x; m.m01 = basis.m01 * scale.x;
	m.m02 = basis.m02 * scale.x; m.m03 = 0.f;

	m.m10 = basis.m10 * scale.y; m.m11 = basis.m11 * scale.y;
	m.m12 = basis.m12 * scale.y; m.m13 = 0.f;

	m.m20 = basis.m20 * scale.z; m.m21 = basis.m21 * scale.z;
	m.m22 = basis.m22 * scale.z; m.m23 = 0.f;

	m.m30 = origin.x;			 m.m31 = origin.y;
	m.m32 = origin.z;			 m.m33 = 1.f;
}

inline void set_basis_scale_origin(Matrix4x4f_a& m,
								   const Matrix4x4f_a& basis,
								   const Vector3f_a& scale,
								   const Vector3f_a& origin) {
	m.m00 = basis.m00 * scale.x; m.m01 = basis.m01 * scale.x;
	m.m02 = basis.m02 * scale.x; m.m03 = 0.f;

	m.m10 = basis.m10 * scale.y; m.m11 = basis.m11 * scale.y;
	m.m12 = basis.m12 * scale.y; m.m13 = 0.f;

	m.m20 = basis.m20 * scale.z; m.m21 = basis.m21 * scale.z;
	m.m22 = basis.m22 * scale.z; m.m23 = 0.f;

	m.m30 = origin.x;			 m.m31 = origin.y;
	m.m32 = origin.z;			 m.m33 = 1.f;
}

inline void set_translation(Matrix4x4f_a& m, const Vector3f_a& v) {
	m.m00 = 1.f; m.m01 = 0.f; m.m02 = 0.f; m.m03 = 0.f;
	m.m10 = 0.f; m.m11 = 1.f; m.m12 = 0.f; m.m13 = 0.f;
	m.m20 = 0.f; m.m21 = 0.f; m.m22 = 1.f; m.m23 = 0.f;
	m.m30 = v.x; m.m31 = v.y; m.m32 = v.z; m.m33 = 1.f;
}

inline void set_rotation_x(Matrix4x4f_a& m, float a) {
	const float c = std::cos(a);
	const float s = std::sin(a);

	m.m00 = 1.f; m.m01 = 0.f; m.m02 =  0.f; m.m03 = 0.f;
	m.m10 = 0.f; m.m11 = c;   m.m12 = -s;   m.m13 = 0.f;
	m.m20 = 0.f; m.m21 = s;   m.m22 =  c;   m.m23 = 0.f;
	m.m30 = 0.f; m.m31 = 0.f; m.m32 =  0.f; m.m33 = 1.f;
}

}
