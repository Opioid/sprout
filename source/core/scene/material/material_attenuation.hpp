#ifndef SU_CORE_SCENE_MATERIAL_ATTENUATION_HPP
#define SU_CORE_SCENE_MATERIAL_ATTENUATION_HPP

#include "base/math/vector3.inl"

namespace scene::material {

static inline float3 extinction_coefficient(float3 const& color, float distance) {
	float3 const ca = math::clamp(color, 0.001f, 0.999f);

	float3 const a = math::log(ca);

	return -a / distance;
}

static inline void attenuation(float3 const& ac, float3 const& ssc, float distance,
							   float3& absorption_coefficient, float3& scattering_coefficient) {
/*	float3 const mu_t = extinction_coefficient(absorption_color, distance);

	float3 const mu_a = mu_t * (1.f - scattering_color);

	absorption_coefficient = mu_a;
	scattering_coefficient = mu_t - mu_a;*/

	float3 const mu_t = extinction_coefficient(ac, distance);

	float3 const root = math::sqrt(9.59217f + 41.6898f * ssc + 17.71226f * ssc * ssc);

	float3 const factor = 4.09712f + 4.20863f * ssc - root;

	float3 const pss = 1.f - (factor * factor);

	float3 const mu_a = mu_t * (1.f - pss);
	absorption_coefficient = mu_a;
	scattering_coefficient = mu_t - mu_a;
}

static inline void attenuation(float3 const& color, float distance,
							   float3& absorption_coefficient, float3& scattering_coefficient) {
	attenuation(color, color, distance, absorption_coefficient, scattering_coefficient);
}

}

#endif
