#pragma once

#include "vector3.hpp"
#include "quaternion.hpp"

namespace math {

/****************************************************************************
 *
 * Generic 3D transformation
 *
 ****************************************************************************/

template<typename T>
struct Transformation {
	Vector3<T>    position;
	Vector3<T>    scale;
	Quaternion<T> rotation;
};

template<typename T>
Transformation<T> lerp(const Transformation<T>& a, const Transformation<T>& b, T t);

// using transformation = Transformation<float>;

/****************************************************************************
 *
 * Aligned 3D float transformation
 *
 ****************************************************************************/

struct alignas(16) Transformationf_a {
	Vector3f_a position;
	Vector3f_a scale;
	quaternion rotation;
};

Transformationf_a lerp(const Transformationf_a& a, const Transformationf_a& b, float t);

using transformation = Transformationf_a;

}
