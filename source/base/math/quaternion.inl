#pragma once

#include "quaternion.hpp"

namespace math {

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
Quaternion<T>& Quaternion<T>::operator*=(const Quaternion<T>& q) {
	Quaternion<T> temp(w * q.x + x * q.w + y * q.z - z * q.y,
					   w * q.y + y * q.w + z * q.x - x * q.z,
					   w * q.z + z * q.w + x * q.y - y * q.x,
					   w * q.w - x * q.x - y * q.y - z * q.z);

	return *this = temp;
}

template<typename T>
const Quaternion<T> Quaternion<T>::identity(T(0), T(0), T(0), T(1));

template<typename T>
T dot(const Quaternion<T>& a, const Quaternion<T>& b) {
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
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
	float cosom = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;

	// adjust signs (if necessary)
	Quaternion<T> end = b;

	if (cosom < T(0)) {
		cosom = -cosom;
		end.x = -end.x;   // Reverse all signs
		end.y = -end.y;
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

	return Quaternion<T>(sclp * a.x + sclq * end.x,
						 sclp * a.y + sclq * end.y,
						 sclp * a.z + sclq * end.z,
						 sclp * a.w + sclq * end.w);
}

}
