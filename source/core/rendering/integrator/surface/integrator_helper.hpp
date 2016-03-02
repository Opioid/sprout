#pragma once

#include "base/math/vector.hpp"
#include <cmath>

namespace rendering {

inline math::float3 attenuation(float distance, math::pfloat3 c) {
	constexpr float e = 2.f;

	return math::float3(std::pow(e, -c.x * distance), std::pow(e, -c.y * distance), std::pow(e, -c.z * distance));
}

inline math::float3 attenuation(math::pfloat3 start, math::pfloat3 end, math::pfloat3 c) {
	return attenuation(math::distance(start, end), c);
}

}
