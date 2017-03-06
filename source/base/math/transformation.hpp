#pragma once

#include "vector3.hpp"
#include "quaternion.hpp"

namespace math {

struct Transformation {
	bool operator==(const Transformation& o) const;

	bool operator!=(const Transformation& o) const;

	Vector3f_a position;
	Vector3f_a scale;
	Quaternion rotation;
};

Transformation lerp(const Transformation& a, const Transformation& b, float t);

}
