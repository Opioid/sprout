#include "density.hpp"
#include "base/math/aabb.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/ray.inl"
#include "base/math/vector3.inl"
#include "base/random/generator.inl"

namespace scene::volume {

float3 Density::emission(const Transformation& /*transformation*/, const math::Ray& /*ray*/,
						 float /*step_size*/, rnd::Generator& /*rng*/,
						 Sampler_filter /*filter*/, const Worker& /*worker*/) const {
	return float3(0.f);
}

float3 Density::optical_depth(const Transformation& transformation, const math::Ray& ray,
							  float step_size, rnd::Generator& rng,
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

	const float3 attenuation = absorption_ + scattering_;

	return step_size * tau * attenuation;
}

float3 Density::scattering(const Transformation& transformation, const float3& p,
						   Sampler_filter filter, const Worker& worker) const {
	const float3 p_o = math::transform_point(p, transformation.world_to_object);
	return density(transformation, p_o, filter, worker) * scattering_;
}

}
