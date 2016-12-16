#pragma once

#include "ray.hpp"
#include "vector3.inl"

namespace math {

inline Ray::Ray(FVector3f_a origin, FVector3f_a direction, float min_t, float max_t) :
	origin(origin),
	direction(direction),
	inv_direction(reciprocal(direction)),
	min_t(min_t),
	max_t(max_t) {
	sign[0] = inv_direction.x < 0.f ? 1 : 0;
	sign[1] = inv_direction.y < 0.f ? 1 : 0;
	sign[2] = inv_direction.z < 0.f ? 1 : 0;
}

inline void Ray::set_direction(FVector3f_a v) {
	direction = v;
	inv_direction = reciprocal(v);

	sign[0] = inv_direction.x < 0.f ? 1 : 0;
	sign[1] = inv_direction.y < 0.f ? 1 : 0;
	sign[2] = inv_direction.z < 0.f ? 1 : 0;
}

inline Vector3f_a Ray::point(float t) const {
	return origin + t * direction;
}

inline float Ray::length() const {
	return math::length((min_t - max_t) * direction);
}

}
