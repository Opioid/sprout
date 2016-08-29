#pragma once

#include "base/math/vector.hpp"
#include <cmath>

namespace rendering {

inline float3 attenuation(float distance, float3_p c) {
	constexpr float e = 2.f;

	return float3(std::pow(e, -c.x * distance),
				  std::pow(e, -c.y * distance),
				  std::pow(e, -c.z * distance));
}

inline float3 attenuation(float3_p start, float3_p end, float3_p c) {
	return attenuation(math::distance(start, end), c);
}

inline float power_heuristic(float fpdf, float gpdf) {
	float f2 = fpdf * fpdf;
	return f2 / (f2 + gpdf * gpdf);
}

}
