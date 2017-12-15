#ifndef SU_CORE_RENDERING_INTEGRATOR_HELPER_HPP
#define SU_CORE_RENDERING_INTEGRATOR_HELPER_HPP

#include "base/math/vector3.inl"
#include <cmath>

namespace rendering {

static inline float3 attenuation(float distance, const float3& c) {
	constexpr float e = 2.f;
	const float nd = -distance;
	return float3(std::pow(e, c[0] * nd),
				  std::pow(e, c[1] * nd),
				  std::pow(e, c[2] * nd));
}

static inline float3 attenuation(const float3& start, const float3& end, const float3& c) {
	return attenuation(math::distance(start, end), c);
}

static inline float balance_heuristic(float f_pdf, float g_pdf) {
	return f_pdf / (f_pdf + g_pdf);
}

static inline float power_heuristic(float f_pdf, float g_pdf) {
	const float f2 = f_pdf * f_pdf;
	return f2 / (f2 + g_pdf * g_pdf);
}

static inline bool russian_roulette(float3& throughput, float continuation_probability, float r) {
	if (r > continuation_probability) {
		return true;
	}

	throughput /= continuation_probability;

	return false;
}

}

#endif
