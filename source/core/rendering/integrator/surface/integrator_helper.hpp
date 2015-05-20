#pragma once

#include "base/math/vector.hpp"
#include <cmath>

namespace rendering {

inline math::float3 attenuation(const math::float3& start, const math::float3& end, const math::float3& c) {
	const float e = 2.f;

	float x = math::distance(start, end);
	return math::float3(std::pow(e, -c.x * x), std::pow(e, -c.y * x), std::pow(e, -c.z * x));
}

}
