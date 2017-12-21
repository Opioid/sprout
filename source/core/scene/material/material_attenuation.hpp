#ifndef SU_CORE_SCENE_MATERIAL_ATTENUATION_HPP
#define SU_CORE_SCENE_MATERIAL_ATTENUATION_HPP

#include "base/math/vector3.inl"

namespace scene::material {

static inline float3 extinction_coefficient(const float3& color, float distance) {
	const float3 ca = math::clamp(color, 0.001f, 0.999f);

	const float3 a = math::log(ca);

	return -a / distance;
}

static inline void attenuation(const float3& absorption_color,
							   const float3& scattering_color,
							   float distance,
							   float3& absorption_coefficient,
							   float3& scattering_coefficient) {
	const float3 sigma_t = extinction_coefficient(absorption_color, distance);

	const float3 sigma_a = sigma_t * (1.f - scattering_color);

	absorption_coefficient = sigma_a;
	scattering_coefficient = sigma_t - sigma_a;
}

}

#endif

