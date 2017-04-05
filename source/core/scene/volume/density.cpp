#include "density.hpp"
#include "base/math/aabb.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/ray.inl"
#include "base/math/vector3.inl"
#include "base/random/generator.inl"

namespace scene { namespace volume {

float3 Density::optical_depth(const math::Ray& ray, float step_size,
							  rnd::Generator& rng, Worker& worker,
							  Sampler_filter filter) const {
	float length = math::length(ray.direction);

	math::Ray rn(ray.origin, ray.direction / length, ray.min_t * length, ray.max_t * length);

	float min_t;
	float max_t;
	if (!aabb_.intersect_p(rn, min_t, max_t)) {
		return float3(0.f);
	}

	min_t += rng.random_float() * step_size;

	float tau = 0.f;

//	math::Ray r_o(math::transform_point(rn.origin, world_transformation_.world_to_object),
//				  math::transform_vector(rn.direction, world_transformation_.world_to_object),
//				  rn.min_t, rn.max_t);

	float3 rp_o = math::transform_point(rn.origin, world_transformation_.world_to_object);
	float3 rd_o = math::transform_vector(rn.direction, world_transformation_.world_to_object);

	for (; min_t < max_t; min_t += step_size) {
		float3 p_o = rp_o + min_t * rd_o; // r_o.point(min_t);
		tau += density(p_o, worker, filter);
	}

	float3 attenuation = absorption_ + scattering_;

	return step_size * tau * attenuation;
}

float3 Density::scattering(const float3& p, Worker& worker, Sampler_filter filter) const {
	float3 p_o = math::transform_point(p, world_transformation_.world_to_object);
	return density(p_o, worker, filter) * scattering_;
}

}}
