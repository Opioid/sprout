#pragma once

#include "transformation.hpp"
#include "vector3.inl"
#include "quaternion.inl"

namespace math {

/****************************************************************************
 *
 * Generic 3D transformation
 *
 ****************************************************************************/

template<typename T>
Transformation<T> lerp(const Transformation<T>& a, const Transformation<T>& b, T t) {
	return Transformation<T>{
		lerp(a.position, b.position, t),
		lerp(a.scale, b.scale, t),
		slerp(a.rotation, b.rotation, t)
	};
}


/****************************************************************************
 *
 * Aligned 3D float transformation
 *
 ****************************************************************************/

inline Transformationf_a lerp(const Transformationf_a& a, const Transformationf_a& b, float t) {
	return Transformationf_a{
		lerp(a.position, b.position, t),
		lerp(a.scale, b.scale, t),
		slerp_quaternion(a.rotation, b.rotation, t)
	};
}

}
