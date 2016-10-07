#include "density.hpp"
#include "base/math/ray.inl"
#include "base/math/vector4.inl"
#include "base/math/bounding/aabb.inl"
#include "base/math/random/generator.inl"

namespace scene { namespace volume {

float3 Density::optical_depth(const math::Oray& ray, float step_size,
							  math::random::Generator& rng) const {
	float length = math::length(ray.direction);

	math::Oray rn(ray.origin, ray.direction / length, ray.min_t * length, ray.max_t * length);

	float min_t;
	float max_t;

	if (!scene_bb_.intersect_p(rn, min_t, max_t)) {
		return float3(0.f);
	}

	min_t += rng.random_float() * step_size;

	float3 attenuation = absorption_ + scattering_;

	float3 tau(0.f);

	for (; min_t < max_t; min_t += step_size) {
		tau += density(rn.point(min_t)) * attenuation;
	}

	return step_size * tau;
}

float3 Density::scattering(float3_p p) const {
	return density(p) * scattering_;
}

}}
