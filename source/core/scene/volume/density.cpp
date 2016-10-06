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

	float t0;
	float t1;

	if (!scene_bb_.intersect_p(rn, t0, t1)) {
		return float3(0.f);
	}

	float3 tau(0.f);
	t0 += rng.random_float() * step_size;

	float3 attenuation = absorption_ + scattering_;

	while (t0 < t1) {
		tau += density(rn.point(t0)) * attenuation;
		t0 += step_size;
	}

	return step_size * tau;
}

float3 Density::scattering(float3_p p) const {
	return density(p) * scattering_;
}

}}
