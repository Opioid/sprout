#pragma once

#include "vector3.hpp"
#include "quaternion.hpp"

namespace math {

struct alignas(16) Transformationf_a {
	bool operator==(const Transformationf_a& o) const;

	bool operator!=(const Transformationf_a& o) const;

	Vector3f_a position;
	Vector3f_a scale;
	Quaternion rotation;
};

Transformationf_a lerp(const Transformationf_a& a, const Transformationf_a& b, float t);

using transformation = Transformationf_a;

}
