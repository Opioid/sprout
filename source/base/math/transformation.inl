#ifndef SU_BASE_MATH_TRANSFORMATION_INL
#define SU_BASE_MATH_TRANSFORMATION_INL

#include "transformation.hpp"
#include "vector3.inl"
#include "quaternion.inl"

namespace math {

inline bool Transformation::operator==(const Transformation& o) const {
	return position == o.position && scale == o.scale && rotation == o.rotation;
}

inline bool Transformation::operator!=(const Transformation& o) const {
	return position != o.position || scale != o.scale || rotation != o.rotation;
}

static inline Transformation lerp(const Transformation& a, const Transformation& b, float t) {
	return Transformation{
		lerp(a.position, b.position, t),
		lerp(a.scale, b.scale, t),
		quaternion::slerp(a.rotation, b.rotation, t)
	};
}

}

#endif
