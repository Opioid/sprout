#pragma once

#include "vector3.hpp"

namespace math {

template<typename T>
struct Plane {
	union {
		struct {
			T a, b, c, d;
		};

		T p[4];
	};

	Plane();

	Plane(T a, T b, T c, T d);

	Plane(const Vector3<T>& normal, T d);

	Plane(const Vector3<T>& normal, const Vector3<T>& point);

	Plane(const Vector3<T>& v0, const Vector3<T>& v1, const Vector3<T>& v2);
};

template<typename T>
T dot(const Plane<T>& p, const Vector3<T>& v);

template<typename T>
bool behind(const Plane<T>& p, const Vector3<T>& point);

typedef Plane<float> plane;

}
