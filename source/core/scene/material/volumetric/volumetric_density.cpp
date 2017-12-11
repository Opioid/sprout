#include "volumetric_density.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/material/material_sample_helper.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/ray.inl"
#include "base/random/generator.inl"

namespace scene::material::volumetric {

Density::Density(const Sampler_settings& sampler_settings) : Material(sampler_settings) {}

float3 Density::emission(const Transformation& /*transformation*/, const math::Ray& /*ray*/,
						 float /*step_size*/, rnd::Generator& /*rng*/,
						 Sampler_filter /*filter*/, const Worker& /*worker*/) const {
	return float3::identity();
}

float3 Density::optical_depth(const Transformation& transformation, const math::AABB& /*aabb*/,
							  const math::Ray& ray, float step_size, rnd::Generator& rng,
							  Sampler_filter filter, const Worker& worker) const {
	const math::Ray rn = ray.normalized();

	float min_t = rn.min_t + rng.random_float() * step_size;
	float tau = 0.f;

	const float3 rp_o = math::transform_point(rn.origin, transformation.world_to_object);
	const float3 rd_o = math::transform_vector(rn.direction, transformation.world_to_object);

	for (; min_t < rn.max_t; min_t += step_size) {
		const float3 p_o = rp_o + min_t * rd_o;
		tau += density(transformation, p_o, filter, worker);
	}

	const float3 attenuation = absorption_coefficient_ + scattering_coefficient_;

	return step_size * tau * attenuation;
}

float3 Density::scattering(const Transformation& transformation, const float3& p,
						   Sampler_filter filter, const Worker& worker) const {
	const float3 p_o = math::transform_point(p, transformation.world_to_object);

	return density(transformation, p_o, filter, worker) * scattering_coefficient_;
}

}

