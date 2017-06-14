#pragma once

#include "base/math/vector3.inl"
#include "base/spectrum/rgb.hpp"
#include <cmath>

namespace rendering {

static inline float3 attenuation(float distance, const float3& c) {
	constexpr float e = 2.f;

	return float3(std::pow(e, -c[0] * distance),
				  std::pow(e, -c[1] * distance),
				  std::pow(e, -c[2] * distance));
}

static inline float3 attenuation(const float3& start, const float3& end, const float3& c) {
	return attenuation(math::distance(start, end), c);
}

static inline float balance_heuristic(float fpdf, float gpdf) {
	return fpdf / (fpdf + gpdf);
}

static inline float power_heuristic(float fpdf, float gpdf) {
	const float f2 = fpdf * fpdf;
	return f2 / (f2 + gpdf * gpdf);
}

static inline bool russian_roulette(float3& throughput, float continuation_probability, float r) {
	const float q = std::max(spectrum::luminance(throughput), continuation_probability);

	if (r > q) {
		return true;
	}

	throughput /= q;

	return false;
}

}
