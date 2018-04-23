#ifndef SU_CORE_SCENE_MATERIAL_ATTENUATION_HPP
#define SU_CORE_SCENE_MATERIAL_ATTENUATION_HPP

#include "base/math/vector3.inl"

namespace scene::material {

static inline float3 extinction_coefficient(const float3& color, float distance) {
	const float3 ca = math::clamp(color, 0.001f, 0.999f);

	const float3 a = math::log(ca);

	return -a / distance;
}

static inline void attenuation(const float3& ac, const float3& ssc, float distance,
							   float3& absorption_coefficient, float3& scattering_coefficient) {
/*	const float3 mu_t = extinction_coefficient(absorption_color, distance);

	const float3 mu_a = mu_t * (1.f - scattering_color);

	absorption_coefficient = mu_a;
	scattering_coefficient = mu_t - mu_a;*/

	const float3 mu_t = extinction_coefficient(ac, distance);

	const float3 root = math::sqrt(9.59217f + 41.6898f * ssc + 17.71226f * ssc * ssc);

	const float3 factor = 4.09712f + 4.20863f * ssc - root;

	const float3 pss = 1.f - (factor * factor);

	const float3 mu_a = mu_t * (1.f - pss);
	absorption_coefficient = mu_a;
	scattering_coefficient = mu_t - mu_a;
}

static inline void attenuation(const float3& color, float distance,
							   float3& absorption_coefficient, float3& scattering_coefficient) {
	attenuation(color, color, distance, absorption_coefficient, scattering_coefficient);
}

}

#endif
