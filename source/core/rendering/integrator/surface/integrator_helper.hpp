#pragma once

#include "base/math/vector.hpp"
#include <cmath>

namespace rendering {

inline math::vec3 attenuation(float distance, math::pvec3 c) {
	constexpr float e = 2.f;

	return math::vec3(std::pow(e, -c.x * distance), std::pow(e, -c.y * distance), std::pow(e, -c.z * distance));
}

inline math::vec3 attenuation(math::pvec3 start, math::pvec3 end, math::pvec3 c) {
	return attenuation(math::distance(start, end), c);
}

}
