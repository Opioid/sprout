#pragma once

#include "vector3.hpp"
#include "vector4.hpp"

namespace math {

/****************************************************************************
 *
 * Generic 3D plane
 *
 ****************************************************************************/

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

/****************************************************************************
 *
 * Aligned 3D float plane
 *
 ****************************************************************************/

//typedef Plane<float> plane;
typedef Vector4f_a plane;

plane create_plane(FVector3f_a normal, float d);
plane create_plane(FVector3f_a normal, FVector3f_a point);
plane create_plane(FVector3f_a v0, FVector3f_a v1, FVector3f_a v2);

float dot(const plane& p, FVector3f_a v);

bool behind(const plane& p, FVector3f_a point);

}
