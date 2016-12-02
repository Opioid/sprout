#include "height.hpp"
#include "base/json/json.hpp"
#include "base/math/vector.inl"
#include "base/math/bounding/aabb.inl"

#include <iostream>
#include "base/math/print.hpp"


namespace scene { namespace volume {

Height::Height() : a_(1.f), b_(1.f) {}

float3 Height::optical_depth(const math::Ray& ray, float step_size,
							 rnd::Generator& rng, Worker& worker,
							 Sampler_filter filter) const {
	float length = math::length(ray.direction);

	math::Ray rn(ray.origin, ray.direction / length, ray.min_t * length, ray.max_t * length);

	float min_t;
	float max_t;
	if (!aabb_.intersect_p(rn, min_t, max_t)) {
		return float3(0.f);
	}

	float3 a = math::transform_point(rn.point(min_t), world_transformation_.world_to_object);
	float3 b = math::transform_point(rn.point(max_t), world_transformation_.world_to_object);

	float ha = 0.5f * (1.f + a.y);
	float hb = 0.5f * (1.f + b.y);

	float3 attenuation = absorption_ + scattering_;

	float3 fa = -((a_ * std::exp(-b_ * ha) * attenuation) / b_);
	float3 fb = -((a_ * std::exp(-b_ * hb) * attenuation) / b_);

	float3 result = (fb - fa) / (hb - ha);

	if (0.f == hb - ha) {
		result = fb;
	}

//	float3 old_result = Density::optical_depth(ray, 0.001f, rng, worker, filter);

//	std::cout << result << " : " << old_result << std::endl;

	return length * result;
}

float Height::density(float3_p p, Worker& /*worker*/, Sampler_filter /*filter*/) const {
	// p is in object space already

	if (!local_aabb_.intersect(p)) {
		return 0.f;
	}

	float height = 0.5f * (1.f + p.y);

	return a_ * std::exp(-b_ * height);
}

void Height::set_parameter(const std::string& name, const json::Value& value) {
	if ("a" == name) {
		a_ = json::read_float(value);
	} else if ("b" == name) {
		b_ = json::read_float(value);
	}
}

}}
