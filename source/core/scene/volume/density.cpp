#include "density.hpp"
#include "base/math/ray.inl"
#include "base/math/matrix.inl"
#include "base/math/vector.inl"
#include "base/math/bounding/aabb.inl"
#include "base/math/random/generator.inl"

namespace scene { namespace volume {

float3 Density::optical_depth(const math::Oray& ray, float step_size,
							  math::random::Generator& rng, Worker& worker,
							  Sampler_filter filter) const {
	float length = math::length(ray.direction);

	math::Oray rn(ray.origin, ray.direction / length, ray.min_t * length, ray.max_t * length);

	float min_t;
	float max_t;
	if (!aabb_.intersect_p(rn, min_t, max_t)) {
		return float3(0.f);
	}

	min_t += rng.random_float() * step_size;

	float3 attenuation = absorption_ + scattering_;

	float3 tau(0.f);

	for (; min_t < max_t; min_t += step_size) {
		float3 p_o = math::transform_point(rn.point(min_t), world_transformation_.world_to_object);
		tau += density(p_o, worker, filter) * attenuation;
	}

	return step_size * tau;
}

float3 Density::scattering(float3_p p, Worker& worker,
						   Sampler_filter filter) const {
	float3 p_o = math::transform_point(p, world_transformation_.world_to_object);
	return density(p_o, worker, filter) * scattering_;
}

}}
