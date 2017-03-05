#pragma once

#include "quaternion.hpp"
#include "matrix3x3.inl"

namespace math {

/****************************************************************************
 *
 * Generic quaternion
 *
 ****************************************************************************/

template<typename T>
Quaternion<T>::Quaternion() {}

template<typename T>
Quaternion<T>::Quaternion(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}

template<typename T>
Quaternion<T>::Quaternion(const Matrix3x3<T>& m) {
	T trace = m.m00 + m.m11 + m.m22;
	T temp[4];

	if (trace > T(0)) {
		T s = std::sqrt(trace + T(1));
		temp[3] = s * T(0.5);
		s = T(0.5) / s;

		temp[0] = (m.m21 - m.m12) * s;
		temp[1] = (m.m02 - m.m20) * s;
		temp[2] = (m.m10 - m.m01) * s;
	} else {
		int i = m.m00 < m.m11 ? (m.m11 < m.m22 ? 2 : 1) : (m.m00 < m.m22 ? 2 : 0);
		int j = (i + 1) % 3;
		int k = (i + 2) % 3;

		T s = std::sqrt(m.m[i * 3 + i] - m.m[j * 3 + j] - m.m[k * 3 + k] + T(1));
		temp[i] = s * T(0.5);
		s = T(0.5) / s;

		temp[3] = (m.m[k * 3 + j] - m.m[j * 3 + k]) * s;
		temp[j] = (m.m[j * 3 + i] + m.m[i * 3 + j]) * s;
		temp[k] = (m.m[k * 3 + i] + m.m[i * 3 + k]) * s;
	}

	x = temp[0];
	y = temp[1];
	z = temp[2];
	w = temp[3];
}

template<typename T>
Quaternion<T> Quaternion<T>::operator*(const Quaternion<T>& q) const {
	return Quaternion<T>(w * q.x + x * q.w + y * q.z - z * q.y,
						 w * q.y + y * q.w + z * q.x - x * q.z,
						 w * q.z + z * q.w + x * q.y - y * q.x,
						 w * q.w - x * q.x - y * q.y - z * q.z);
}

template<typename T>
const Quaternion<T> Quaternion<T>::identity(T(0), T(0), T(0), T(1));

template<typename T>
T dot(const Quaternion<T>& a, const Quaternion<T>& b) {
	return (a.x * b.x + a.y * b.y) + (a.z * b.z + a.w * b.w);
}

template<typename T>
T length(const Quaternion<T>& q) {
	return std::sqrt(dot(q, q));
}

template<typename T>
T angle(const Quaternion<T>& a, const Quaternion<T>& b) {
	T s = std::sqrt(dot(a, a) * dot(b, b));
	return std::acos(dot(a, b) / s);
}

template<typename T>
void set_rotation_x(Quaternion<T>& q, T a) {
	q.x = std::sin(a * T(0.5));
	q.y = T(0);
	q.z = T(0);
	q.w = std::cos(a * T(0.5));
}

template<typename T>
void set_rotation_y(Quaternion<T>& q, T a) {
	q.x = T(0);
	q.y = std::sin(a * T(0.5));
	q.z = T(0);
	q.w = std::cos(a * T(0.5));
}

template<typename T>
void set_rotation_z(Quaternion<T>& q, T a) {
	q.x = T(0);
	q.y = T(0);
	q.z = std::sin(a * T(0.5));
	q.w = std::cos(a * T(0.5));
}

template<typename T>
void set_rotation(Quaternion<T>& q, const Vector3<T>& v, T a) {
	const T d = length(v);

	const T s = std::sin(a * T(0.5)) / d;

	q.x = v.x * s;
	q.y = v.y * s;
	q.z = v.z * s;
	q.w = cos(a * T(0.5));
}

template<typename T>
void set_rotation(Quaternion<T>& q, T yaw, T pitch, T roll) {
	const T half_yaw   = yaw   * T(0.5);
	const T half_pitch = pitch * T(0.5);
	const T half_roll  = roll  * T(0.5);

	const T cos_yaw   = std::cos(half_yaw);
	const T sin_yaw   = std::sin(half_yaw);
	const T cos_pitch = std::cos(half_pitch);
	const T sin_pitch = std::sin(half_pitch);
	const T cos_roll  = std::cos(half_roll);
	const T sin_roll  = std::sin(half_roll);

	q.x = cos_roll * sin_pitch * cos_yaw + sin_roll * cos_pitch * sin_yaw;
	q.y = cos_roll * cos_pitch * sin_yaw - sin_roll * sin_pitch * cos_yaw;
	q.z = sin_roll * cos_pitch * cos_yaw - cos_roll * sin_pitch * sin_yaw;
	q.w = cos_roll * cos_pitch * cos_yaw + sin_roll * sin_pitch * sin_yaw;
}

template<typename T>
Quaternion<T> slerp(const Quaternion<T>& a, const Quaternion<T>& b, T t) {
	// calc cosine theta
	T cosom = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;

	// adjust signs (if necessary)
	Quaternion<T> end = b;

	if (cosom < T(0)) {
		cosom = -cosom;
		end[0] = -end[0];   // Reverse all signs
		end[1] = -end[1];
		end.z = -end.z;
		end.w = -end.w;
	}

	// Calculate coefficients
	float sclp;
	float sclq;

	// 0.0001 -> some epsillon
	if (T(1) - cosom > T(0.0001)) {
		// Standard case (slerp)
		float omega = std::acos(cosom); // extract theta from dot product's cos theta
		float sinom = std::sin(omega);
		sclp  = std::sin((T(1) - t) * omega) / sinom;
		sclq  = std::sin(t * omega) / sinom;
	} else {
		// Very close, do linear interpolation (because it's faster)
		sclp = T(1) - t;
		sclq = t;
	}

	return Quaternion<T>(sclp * a.x + sclq * end[0],
						 sclp * a.y + sclq * end[1],
						 sclp * a.z + sclq * end.z,
						 sclp * a.w + sclq * end.w);
}

/****************************************************************************
 *
 * Aligned quaternon functions
 *
 ****************************************************************************/

inline quaternion create_quaternion(const Matrix3x3<float>& m) {
	float trace = m.m00 + m.m11 + m.m22;
	quaternion temp;

	if (trace > 0.f) {
		float s = std::sqrt(trace + 1.f);
		temp[3] = s * 0.5f;
		s = 0.5f / s;

		temp[0] = (m.m21 - m.m12) * s;
		temp[1] = (m.m02 - m.m20) * s;
		temp[2] = (m.m10 - m.m01) * s;
	} else {
		int i = m.m00 < m.m11 ? (m.m11 < m.m22 ? 2 : 1) : (m.m00 < m.m22 ? 2 : 0);
		int j = (i + 1) % 3;
		int k = (i + 2) % 3;

		float s = std::sqrt(m.m[i * 3 + i] - m.m[j * 3 + j] - m.m[k * 3 + k] + 1.f);
		temp.v[i] = s * 0.5f;
		s = 0.5f / s;

		temp[3] = (m.m[k * 3 + j] - m.m[j * 3 + k]) * s;
		temp.v[j] = (m.m[j * 3 + i] + m.m[i * 3 + j]) * s;
		temp.v[k] = (m.m[k * 3 + i] + m.m[i * 3 + k]) * s;
	}

	return temp;
}

inline quaternion create_quaternion(const Matrix3x3f_a& m) {
	float trace = m.r[0][0] + m.r[1][1] + m.r[2][2];
	quaternion temp;

	if (trace > 0.f) {
		float s = std::sqrt(trace + 1.f);
		temp[3] = s * 0.5f;
		s = 0.5f / s;

		temp[0] = (m.r[2][1] - m.r[1][2]) * s;
		temp[1] = (m.r[0][2] - m.r[2][0]) * s;
		temp[2] = (m.r[1][0] - m.r[0][1]) * s;
	} else {
		int i = m.r[0][0] < m.r[1][1] ? (m.r[1][1] < m.r[2][2] ? 2 : 1) : (m.r[0][0] < m.r[2][2] ? 2 : 0);
		int j = (i + 1) % 3;
		int k = (i + 2) % 3;

		float s = std::sqrt(m.r[i].v[i] - m.r[j].v[j] - m.r[k].v[k] + 1.f);
		temp.v[i] = s * 0.5f;
		s = 0.5f / s;

		temp[3] = (m.r[k].v[j] - m.r[j].v[k]) * s;
		temp.v[j] = (m.r[j].v[i] + m.r[i].v[j]) * s;
		temp.v[k] = (m.r[k].v[i] + m.r[i].v[k]) * s;
	}

	return temp;
}

inline quaternion create_quaternion_rotation_x(float a) {
	return quaternion(std::sin(a * 0.5f),
					  0.f,
					  0.f,
					  std::cos(a * 0.5f));
}

inline quaternion create_quaternion_rotation_y(float a) {
	return quaternion(0.f,
					  std::sin(a * 0.5f),
					  0.f,
					  std::cos(a * 0.5f));
}

inline quaternion create_quaternion_rotation_z(float a) {
	return quaternion(0.f,
					  0.f,
					  std::sin(a * 0.5f),
					  std::cos(a * 0.5f));
}

inline quaternion mul_quaternion(const quaternion& a, const quaternion& b) {
	return quaternion((a[3] * b[0] + a[0] * b[3]) + (a[1] * b[2] - a[2] * b[1]),
					  (a[3] * b[1] + a[1] * b[3]) + (a[2] * b[0] - a[0] * b[2]),
					  (a[3] * b[2] + a[2] * b[3]) + (a[0] * b[1] - a[1] * b[0]),
					  (a[3] * b[3] - a[0] * b[0]) - (a[1] * b[1] + a[2] * b[2]));
}

inline quaternion slerp_quaternion(const quaternion& a, const quaternion& b, float t) {
	// calc cosine theta
	float cosom = (a[0] * b[0] + a[1] * b[1]) + (a[2] * b[2] + a[3] * b[3]);

	// adjust signs (if necessary)
	quaternion end = b;

	if (cosom < 0.f) {
		cosom = -cosom;
		end[0] = -end[0];   // Reverse all signs
		end[1] = -end[1];
		end[2] = -end[2];
		end[3] = -end[3];
	}

	// Calculate coefficients
	float sclp;
	float sclq;

	// 0.0001 -> some epsillon
	if (1.f - cosom > 0.0001f) {
		// Standard case (slerp)
		float omega = std::acos(cosom); // extract theta from dot product's cos theta
		float sinom = std::sin(omega);
		sclp  = std::sin((1.f - t) * omega) / sinom;
		sclq  = std::sin(t * omega) / sinom;
	} else {
		// Very close, do linear interpolation (because it's faster)
		sclp = 1.f - t;
		sclq = t;
	}

	return quaternion(sclp * a[0] + sclq * end[0],
					  sclp * a[1] + sclq * end[1],
					  sclp * a[2] + sclq * end[2],
					  sclp * a[3] + sclq * end[3]);
}

}
