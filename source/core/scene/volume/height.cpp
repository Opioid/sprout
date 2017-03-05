#include "height.hpp"
#include "base/json/json.hpp"
#include "base/math/aabb.inl"
#include "base/math/ray.inl"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"

#include "base/debug/assert.hpp"

namespace scene { namespace volume {

Height::Height() : a_(1.f), b_(1.f) {}

float3 Height::optical_depth(const math::Ray& ray, float /*step_size*/,
							 rnd::Generator& /*rng*/, Worker& /*worker*/,
							 Sampler_filter /*filter*/) const {
	float length = math::length(ray.direction);

	math::Ray rn(ray.origin, ray.direction / length, ray.min_t * length, ray.max_t * length);

	float min_t;
	float max_t;
	if (!aabb_.intersect_p(rn, min_t, max_t)) {
		return float3(0.f);
	}

	// This is an optimization of the generic stochastic method
	// implemented in Density::opptical_depth.
	// Because everything happens in world space there could be differences
	// when the volume is rotated because the local aabb is never checked.

	float3 a = rn.point(min_t);
	float3 b = rn.point(max_t);

	float min_y = aabb_.min()[1];
	float ha = a[1] - min_y;
	float hb = b[1] - min_y;

	float3 attenuation = absorption_ + scattering_;

	float d = max_t - min_t;

	if (ha == hb) {
		// special case where density stays exactly the same along the ray
		float3 result = d * (a_ * std::exp(-b_ * ha)) * attenuation;

		SOFT_ASSERT(math::all_finite(result));

		return result;
	}

	// calculate the integral of the exponential density function
//	float fa = -((a_ * std::exp(-b_ * ha)) / b_);
//	float fb = -((a_ * std::exp(-b_ * hb)) / b_);

//	float3 result = d * ((fb - fa) / (hb - ha)) * attenuation;

	float fa = -std::exp(-b_ * ha);
	float fb = -std::exp(-b_ * hb);

	float3 result = d * ((a_ * (fb - fa) / b_) / (hb - ha)) * attenuation;

	SOFT_ASSERT(math::all_finite(result));

	return result;

//	float3 old_result = Density::optical_depth(ray, step_size, rng, worker, filter);
//	return old_result;
}

float Height::density(float3_p p, Worker& /*worker*/, Sampler_filter /*filter*/) const {
	// p is in object space already
	if (!local_aabb_.intersect(p)) {
		return 0.f;
	}

	// calculate height, relative to volume, in world space
	float height = world_transformation_.scale[1] * (1.f + p[1]);

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
