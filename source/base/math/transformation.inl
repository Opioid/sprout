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

inline bool Transformationf_a::operator==(const Transformationf_a& o) const {
	return position == o.position && scale == o.scale && rotation == o.rotation;
}

inline bool Transformationf_a::operator!=(const Transformationf_a& o) const {
	return position != o.position || scale != o.scale || rotation != o.rotation;
}

inline Transformationf_a lerp(const Transformationf_a& a, const Transformationf_a& b, float t) {
	return Transformationf_a{
		lerp(a.position, b.position, t),
		lerp(a.scale, b.scale, t),
		quat::slerp(a.rotation, b.rotation, t)
	};
}

}
