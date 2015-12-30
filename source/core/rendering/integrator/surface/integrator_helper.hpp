#pragma once

#include "base/math/vector.hpp"
#include <cmath>

namespace rendering {

inline math::float3 attenuation(float distance, const math::float3& c) {
	const float e = 2.f;

	return math::float3(std::pow(e, -c.x * distance), std::pow(e, -c.y * distance), std::pow(e, -c.z * distance));
}

inline math::float3 attenuation(const math::float3& start, const math::float3& end, const math::float3& c) {
	return attenuation(math::distance(start, end), c);
}

}
