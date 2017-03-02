#pragma once

#include "vector3.hpp"
#include <cstdint>

namespace math {

struct Ray {
	Ray() = default;
	Ray(FVector3f_a origin, FVector3f_a direction,
		float min_t = 0.f, float max_t = 1.f);

	void set_direction(FVector3f_a v);

	Vector3f_a point(float t) const;

	float length() const;

	Vector3f_a origin;
	Vector3f_a direction;
	Vector3f_a inv_direction;
	float min_t;
	float max_t;
	int8_t signs[3];
};

}
