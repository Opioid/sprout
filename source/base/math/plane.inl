#pragma once

#include "plane.hpp"
#include "vector3.inl"

namespace math {

template<typename T>
Plane<T>::Plane() {}

template<typename T>
Plane<T>::Plane(T a, T b, T c, T d) : a(a), b(b), c(c), d(d) {}

template<typename T>
Plane<T>::Plane(const Vector3<T>& normal, T d) : a(normal.x), b(normal.y), c(normal.z), d(d) {}

template<typename T>
Plane<T>::Plane(const Vector3<T>& normal, const Vector3<T>& point) :
	a(normal.x), b(normal.y), c(normal.z), d(-dot(normal, point)) {}

template<typename T>
Plane<T>::Plane(const Vector3<T>& v0, const Vector3<T>& v1, const Vector3<T>& v2) {
	Vector3<T> n = normalized(cross(v2 - v1, v0 - v1));
	a = n.x;
	b = n.y;
	c = n.z;

	d = -dot(n, v0);
}

template<typename T>
T dot(const Plane<T>& p, const Vector3<T>& v) {
	return p.a * v.x + p.b * v.y + p.c * v.z + p.d;
}

template<typename T>
bool behind(const Plane<T>& p, const Vector3<T>& point) {
	return dot(p, point) < 0.f;
}

}
