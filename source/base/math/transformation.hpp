#pragma once

#include "vector3.hpp"
#include "quaternion.hpp"

namespace math {

template<typename T> struct Vector3;
template<typename T> struct Quaternion;

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

// typedef Transformation<float> transformation;

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

typedef Transformationf_a transformation;

}
