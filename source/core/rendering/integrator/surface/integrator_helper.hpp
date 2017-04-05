#pragma once

#include "base/math/vector3.inl"
#include <cmath>

namespace rendering {

inline float3 attenuation(float distance, const float3& c) {
	constexpr float e = 2.f;

	return float3(std::pow(e, -c[0] * distance),
				  std::pow(e, -c[1] * distance),
				  std::pow(e, -c[2] * distance));
}

inline float3 attenuation(const float3& start, const float3& end, const float3& c) {
	return attenuation(math::distance(start, end), c);
}

inline float balance_heuristic(float fpdf, float gpdf) {
	return fpdf / (fpdf + gpdf);
}

inline float power_heuristic(float fpdf, float gpdf) {
	float f2 = fpdf * fpdf;
	return f2 / (f2 + gpdf * gpdf);
}

}
