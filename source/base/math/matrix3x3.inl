#pragma once

#include "matrix3x3.hpp"
#include "vector4.inl"
#include <cmath>

namespace math {

template<typename T>
Matrix3x3<T>::Matrix3x3() {}

template<typename T>
Matrix3x3<T>::Matrix3x3(T m00, T m01, T m02,
                        T m10, T m11, T m12,
                        T m20, T m21, T m22):
	m00(m00), m01(m01), m02(m02),
	m10(m10), m11(m11), m12(m12),
	m20(m20), m21(m21), m22(m22) {}

template<typename T>
Matrix3x3<T>::Matrix3x3(const Vector3<T>& x, const Vector3<T>& y, const Vector3<T>& z) :
    x(x), y(y), z(z) {}

template<typename T>
Matrix3x3<T>::Matrix3x3(const Matrix4x4<T>& m) :
	m00(m.m00), m01(m.m01), m02(m.m02),
	m10(m.m10), m11(m.m11), m12(m.m12),
	m20(m.m20), m21(m.m21), m22(m.m22) {}

template<typename T>
Matrix3x3<T>::Matrix3x3(const Quaternion<T>& q) {
	T d = dot(q, q);

	T s = T(2) / d;

	T xs = q.x * s,  ys = q.y * s,  zs = q.z * s;
	T wx = q.w * xs, wy = q.w * ys, wz = q.w * zs;
	T xx = q.x * xs, xy = q.x * ys, xz = q.x * zs;
	T yy = q.y * ys, yz = q.y * zs, zz = q.z * zs;

	m00 = T(1) - (yy + zz); m01 = xy - wz;          m02 = xz + wy;
	m10 = xy + wz;          m11 = T(1) - (xx + zz); m12 = yz - wx;
	m20 = xz - wy;          m21 = yz + wx,          m22 = T(1) - (xx + yy);
}

template<typename T>
Matrix3x3<T> Matrix3x3<T>::operator*(const Matrix3x3& m) const {
	return Matrix3x3(m00 * m.m00 + m01 * m.m10 + m02 * m.m20,
					 m00 * m.m01 + m01 * m.m11 + m02 * m.m21,
					 m00 * m.m02 + m01 * m.m12 + m02 * m.m22,

					 m10 * m.m00 + m11 * m.m10 + m12 * m.m20,
					 m10 * m.m01 + m11 * m.m11 + m12 * m.m21,
					 m10 * m.m02 + m11 * m.m12 + m12 * m.m22,

					 m20 * m.m00 + m21 * m.m10 + m22 * m.m20,
					 m20 * m.m01 + m21 * m.m11 + m22 * m.m21,
					 m20 * m.m02 + m21 * m.m12 + m22 * m.m22);
}

template<typename T>
Matrix3x3<T> Matrix3x3<T>::operator/(T s) const {
	T is = T(1) / s;
	return Matrix3x3<T>(is * m00, is * m01, is * m02,
						is * m10, is * m11, is * m12,
						is * m20, is * m21, is * m22);
}

template<typename T>
Matrix3x3<T>& Matrix3x3<T>::operator*=(const Matrix3x3& m) {
	Matrix3x3 temp(m00 * m.m00 + m01 * m.m10 + m02 * m.m20,
				   m00 * m.m01 + m01 * m.m11 + m02 * m.m21,
				   m00 * m.m02 + m01 * m.m12 + m02 * m.m22,

				   m10 * m.m00 + m11 * m.m10 + m12 * m.m20,
				   m10 * m.m01 + m11 * m.m11 + m12 * m.m21,
				   m10 * m.m02 + m11 * m.m12 + m12 * m.m22,

				   m20 * m.m00 + m21 * m.m10 + m22 * m.m20,
				   m20 * m.m01 + m21 * m.m11 + m22 * m.m21,
				   m20 * m.m02 + m21 * m.m12 + m22 * m.m22);

	return *this = temp;
}

template<typename T>
Matrix3x3<T> Matrix3x3<T>::identity() {
	return Matrix3x3<T>(T(1), T(0), T(0),
						T(0), T(1), T(0),
						T(0), T(0), T(1));
}

template<typename T>
Vector3<T> operator*(const Vector3<T>& v, const Matrix3x3<T>& m) {
	return Vector3<T>(v.x * m.m00 + v.y * m.m10 + v.z * m.m20,
					  v.x * m.m01 + v.y * m.m11 + v.z * m.m21,
					  v.x * m.m02 + v.y * m.m12 + v.z * m.m22);
}

inline Vector3f_a operator*(FVector3f_a v, const Matrix3x3<float>& m) {
	return Vector3f_a(v.x * m.m00 + v.y * m.m10 + v.z * m.m20,
					  v.x * m.m01 + v.y * m.m11 + v.z * m.m21,
					  v.x * m.m02 + v.y * m.m12 + v.z * m.m22);
}

template<typename T>
Vector3<T>& operator*=(Vector3<T>& v, const Matrix3x3<T>& m) {
	Vector3<T> temp(v.x * m.m00 + v.y * m.m10 + v.z * m.m20,
					v.x * m.m01 + v.y * m.m11 + v.z * m.m21,
					v.x * m.m02 + v.y * m.m12 + v.z * m.m22);

	return v = temp;
}

template<typename T>
Vector3<T> transform_vector(const Matrix3x3<T>& m, const Vector3<T>& v) {
	return Vector3<T>(v.x * m.m00 + v.y * m.m10 + v.z * m.m20,
					  v.x * m.m01 + v.y * m.m11 + v.z * m.m21,
					  v.x * m.m02 + v.y * m.m12 + v.z * m.m22);
}

template<typename T>
Vector3<T> transform_vector_transposed(const Matrix3x3<T>& m, const Vector3<T>& v) {
	return Vector3<T>(v.x * m.m00 + v.y * m.m01 + v.z * m.m02,
					  v.x * m.m10 + v.y * m.m11 + v.z * m.m12,
					  v.x * m.m20 + v.y * m.m21 + v.z * m.m22);
}

template<typename T>
void transform_vectors(const Matrix3x3<T>& m,
                       const Vector3<T>& a, const Vector3<T>& b, Vector3<T>& oa, Vector3<T>& ob) {
	oa.x = a.x * m.m00 + a.y * m.m10 + a.z * m.m20;
	oa.y = a.x * m.m01 + a.y * m.m11 + a.z * m.m21;
	oa.z = a.x * m.m02 + a.y * m.m12 + a.z * m.m22;

	ob.x = b.x * m.m00 + b.y * m.m10 + b.z * m.m20;
	ob.y = b.x * m.m01 + b.y * m.m11 + b.z * m.m21;
	ob.z = b.x * m.m02 + b.y * m.m12 + b.z * m.m22;
}

template<typename T>
Matrix3x3<T> normalized(const Matrix3x3<T>& m) {
	T s0 = T(1) / length(Vector3<T>(m.m00, m.m01, m.m02));
	T s1 = T(1) / length(Vector3<T>(m.m10, m.m11, m.m12));
	T s2 = T(1) / length(Vector3<T>(m.m20, m.m21, m.m22));

	return Matrix3x3<T>(s0 * m.m00, s0 * m.m01, s0 * m.m02,
						s1 * m.m10, s1 * m.m11, s1 * m.m12,
						s2 * m.m20, s2 * m.m21, s2 * m.m22);
}

template<typename T>
T det(const Matrix3x3<T>& m) {
	return m.m00 * m.m11 * m.m22
		 + m.m10 * m.m21 * m.m02
		 + m.m20 * m.m01 * m.m12
		 - m.m00 * m.m21 * m.m12
		 - m.m20 * m.m11 * m.m02
		 - m.m10 * m.m01 * m.m22;
}

template<typename T>
Matrix3x3<T> invert(const Matrix3x3<T>& m) {
	return Matrix3x3<T>(m.m11 * m.m22 - m.m12 * m.m21, m.m02 * m.m21 - m.m01 * m.m22, m.m01 * m.m12 - m.m02 * m.m11,
						m.m12 * m.m20 - m.m10 * m.m22, m.m00 * m.m22 - m.m02 * m.m20, m.m02 * m.m10 - m.m00 * m.m12,
						m.m10 * m.m21 - m.m11 * m.m20, m.m01 * m.m20 - m.m00 * m.m21, m.m00 * m.m11 - m.m01 * m.m10) / det(m);
}

template<typename T>
void set_look_at(Matrix3x3<T>& m, const Vector3<T>& dir, const Vector3<T>& up) {
	Vector3<T> z = normalize(dir);
	Vector3<T> x = normalize(cross(up, z));
	Vector3<T> y = cross(z, x);

	/*
	m.m00 =  x.x;		  m.m01 = y.x;		    m.m02 = z.x;
	m.m10 =  x.y;         m.m11 = y.y;          m.m12 = z.y;
	m.m20 =  x.z;         m.m21 = y.z;          m.m22 = z.z;
	*/
	//Confusion!

	m.m00 =  x.x;		  m.m01 = x.y;		    m.m02 = x.z;
	m.m10 =  y.x;         m.m11 = y.y;          m.m12 = y.z;
	m.m20 =  z.x;         m.m21 = z.y;          m.m22 = z.z;
}

/*
template<typename T>
void set_basis(Matrix3x3<T>& m, const Vector3<T>& v) {
	m.rows[2] = v;

	if (v.x < T(0.6) && v.x > -T(0.6)) {
		m.rows[1] = Vector3<T>(T(1), T(0), T(0));
	} else if (v.y < T(0.6) && v.y > T(0.6)) {
		m.rows[1] = Vector3<T>(T(0), T(1), T(0));
	} else {
		m.rows[1] = Vector3<T>(T(0), T(0), T(1));
	}

	m.rows[0] = normalized(cross(v, m.rows[1]));
	m.rows[1] = cross(m.rows[0], m.rows[2]);
}*/

template<typename T>
void set_scale(Matrix3x3<T>& m, T x, T y, T z) {
	m.m00 = x;    m.m01 = T(0); m.m02 = T(0);
	m.m10 = T(0); m.m11 = y;    m.m12 = T(0);
	m.m20 = T(0); m.m21 = T(0); m.m22 = z;
}

template<typename T>
void set_scale(Matrix3x3<T>& m, const Vector3<T>& v) {
	m.m00 = v.x;  m.m01 = T(0); m.m02 = T(0);
	m.m10 = T(0); m.m11 = v.y;  m.m12 = T(0);
	m.m20 = T(0); m.m21 = T(0); m.m22 = v.z;
}

template<typename T>
void scale(Matrix3x3<T>& m, const Vector3<T>& v) {
	m.m00 *= v.x; m.m01 *= v.x; m.m02 *= v.x;
	m.m10 *= v.y; m.m11 *= v.y; m.m12 *= v.y;
	m.m20 *= v.z; m.m21 *= v.z; m.m22 *= v.z;
}

template<typename T>
void set_rotation_x(Matrix3x3<T>& m, T a) {
	T c = std::cos(a);
	T s = std::sin(a);

	m.m00 = T(1); m.m01 = T(0); m.m02 = T(0);
	m.m10 = T(0); m.m11 = c;    m.m12 = -s;
	m.m20 = T(0); m.m21 = s;    m.m22 =  c;
}

template<typename T>
void set_rotation_y(Matrix3x3<T>& m, T a) {
	T c = std::cos(a);
	T s = std::sin(a);

	m.m00 =  c;    m.m01 = T(0); m.m02 = s;
	m.m10 =  T(0); m.m11 = T(1); m.m12 = T(0);
	m.m20 = -s;    m.m21 = T(0); m.m22 = c;
}

template<typename T>
void set_rotation_z(Matrix3x3<T>& m, T a) {
	T c = std::cos(a);
	T s = std::sin(a);

	m.m00 = c;    m.m01 = -s;    m.m02 = T(0);
	m.m10 = s;    m.m11 =  c;    m.m12 = T(0);
	m.m20 = T(0); m.m21 =  T(0); m.m22 = T(1);
}

template<typename T>
void set_rotation(Matrix3x3<T>& m, const Vector3<T>& v, T a) {
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
}

template<typename T>
Matrix3x3<T> transposed(const Matrix3x3<T>& m) {
	return Matrix3x3<T>(m.m00, m.m10, m.m20,
						m.m01, m.m11, m.m21,
						m.m02, m.m12, m.m22);
}

inline Matrix3x3f_a::Matrix3x3f_a() {}

inline Matrix3x3f_a::Matrix3x3f_a(float m00, float m01, float m02,
								  float m10, float m11, float m12,
								  float m20, float m21, float m22) :
	m00(m00), m01(m01), m02(m02),
	m10(m10), m11(m11), m12(m12),
	m20(m20), m21(m21), m22(m22) {}

inline Matrix3x3f_a Matrix3x3f_a::operator*(const Matrix3x3f_a& a) const {
	return Matrix3x3f_a(m00 * a.m00 + m01 * a.m10 + m02 * a.m20,
						m00 * a.m01 + m01 * a.m11 + m02 * a.m21,
						m00 * a.m02 + m01 * a.m12 + m02 * a.m22,

						m10 * a.m00 + m11 * a.m10 + m12 * a.m20,
						m10 * a.m01 + m11 * a.m11 + m12 * a.m21,
						m10 * a.m02 + m11 * a.m12 + m12 * a.m22,

						m20 * a.m00 + m21 * a.m10 + m22 * a.m20,
						m20 * a.m01 + m21 * a.m11 + m22 * a.m21,
						m20 * a.m02 + m21 * a.m12 + m22 * a.m22);
}

inline Matrix3x3f_a& Matrix3x3f_a::operator*=(const Matrix3x3f_a& a) {
	Matrix3x3f_a temp(m00 * a.m00 + m01 * a.m10 + m02 * a.m20,
					  m00 * a.m01 + m01 * a.m11 + m02 * a.m21,
					  m00 * a.m02 + m01 * a.m12 + m02 * a.m22,

					  m10 * a.m00 + m11 * a.m10 + m12 * a.m20,
					  m10 * a.m01 + m11 * a.m11 + m12 * a.m21,
					  m10 * a.m02 + m11 * a.m12 + m12 * a.m22,

					  m20 * a.m00 + m21 * a.m10 + m22 * a.m20,
					  m20 * a.m01 + m21 * a.m11 + m22 * a.m21,
					  m20 * a.m02 + m21 * a.m12 + m22 * a.m22);

	return *this = temp;
}

inline Matrix3x3f_a Matrix3x3f_a::identity() {
	return Matrix3x3f_a(1.f, 0.f, 0.f,
						0.f, 1.f, 0.f,
						0.f, 0.f, 1.f);
}

inline Matrix3x3f_a create_matrix3x3(const Vector4f_a& q) {
	float d = dot(q, q);

	float s = 2.f / d;

	float xs = q.x * s,  ys = q.y * s,  zs = q.z * s;
	float wx = q.w * xs, wy = q.w * ys, wz = q.w * zs;
	float xx = q.x * xs, xy = q.x * ys, xz = q.x * zs;
	float yy = q.y * ys, yz = q.y * zs, zz = q.z * zs;

	return Matrix3x3f_a(1.f - (yy + zz), xy - wz,         xz + wy,
						xy + wz,         1.f - (xx + zz), yz - wx,
						xz - wy,         yz + wx,         1.f - (xx + yy));
}

inline Vector3f_a operator*(FVector3f_a v, const Matrix3x3f_a& m) {
	return Vector3f_a(v.x * m.m00 + v.y * m.m10 + v.z * m.m20,
					  v.x * m.m01 + v.y * m.m11 + v.z * m.m21,
					  v.x * m.m02 + v.y * m.m12 + v.z * m.m22);
}

inline Vector3f_a transform_vector(FVector3f_a v, const Matrix3x3f_a& m) {
	return Vector3f_a(v.x * m.m00 + v.y * m.m10 + v.z * m.m20,
					  v.x * m.m01 + v.y * m.m11 + v.z * m.m21,
					  v.x * m.m02 + v.y * m.m12 + v.z * m.m22);
}

inline Vector3f_a transform_vector_transposed(FVector3f_a v, const Matrix3x3f_a& m) {
	return Vector3f_a(v.x * m.m00 + v.y * m.m01 + v.z * m.m02,
					  v.x * m.m10 + v.y * m.m11 + v.z * m.m12,
					  v.x * m.m20 + v.y * m.m21 + v.z * m.m22);
}

inline void set_rotation_x(Matrix3x3f_a& m, float a) {
	float c = std::cos(a);
	float s = std::sin(a);

	m.m00 = 1.f; m.m01 = 0.f; m.m02 = 0.f;
	m.m10 = 0.f; m.m11 = c;    m.m12 = -s;
	m.m20 = 0.f; m.m21 = s;    m.m22 =  c;
}

inline void set_rotation_y(Matrix3x3f_a& m, float a) {
	float c = std::cos(a);
	float s = std::sin(a);

	m.m00 =  c;   m.m01 = 0.f; m.m02 = s;
	m.m10 =  0.f; m.m11 = 1.f; m.m12 = 0.f;
	m.m20 = -s;   m.m21 = 0.f; m.m22 = c;
}

inline void set_rotation_z(Matrix3x3f_a& m, float a) {
	float c = std::cos(a);
	float s = std::sin(a);

	m.m00 = c;   m.m01 = -s;   m.m02 = 0.f;
	m.m10 = s;   m.m11 =  c;   m.m12 = 0.f;
	m.m20 = 0.f; m.m21 =  0.f; m.m22 = 1.f;
}

inline void set_rotation(Matrix3x3f_a& m, const Vector3f_a& v, float a) {
	float c = std::cos(a);
	float s = std::sin(a);
	float t = 1.f - c;

	m.m00 = c + v.x * v.x * t;
	m.m11 = c + v.y * v.y * t;
	m.m22 = c + v.z * v.z * t;

	float tmp1 = v.x * v.y * t;
	float tmp2 = v.z * s;

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
}

}
