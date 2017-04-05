#pragma once

#include "vector3.inl"

namespace math {

struct Ray {
	Ray() = default;
	Ray(const Vector3f_a& origin, const Vector3f_a& direction,
		float min_t = 0.f, float max_t = 1.f) :
		origin(origin),
		direction(direction),
		inv_direction(reciprocal(direction)),
		min_t(min_t),
		max_t(max_t) {
		signs[0] = inv_direction[0] < 0.f ? 1 : 0;
		signs[1] = inv_direction[1] < 0.f ? 1 : 0;
		signs[2] = inv_direction[2] < 0.f ? 1 : 0;
	}

	void set_direction(const Vector3f_a& v) {
		direction = v;
		inv_direction = reciprocal(v);

		signs[0] = inv_direction[0] < 0.f ? 1 : 0;
		signs[1] = inv_direction[1] < 0.f ? 1 : 0;
		signs[2] = inv_direction[2] < 0.f ? 1 : 0;
	}

	Vector3f_a point(float t) const {
		return origin + t * direction;
	}

	float length() const {
		return math::length((min_t - max_t) * direction);
	}

	Vector3f_a origin;
	Vector3f_a direction;
	Vector3f_a inv_direction;
	float min_t;
	float max_t;
	uint8_t signs[3];
};

}
