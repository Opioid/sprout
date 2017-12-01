#include "height.hpp"
#include "base/json/json.hpp"
#include "base/math/aabb.inl"
#include "base/math/exp.hpp"
#include "base/math/vector3.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/ray.inl"

#include "base/debug/assert.hpp"

namespace scene::volume {

float3 Height::optical_depth(const Transformation& /*transformation*/, const math::Ray& ray,
							 float /*step_size*/, rnd::Generator& /*rng*/,
							 Sampler_filter /*filter*/, const Worker& /*worker*/) const {
	const math::Ray rn = ray.normalized();

	// This is an optimization of the generic stochastic method
	// implemented in Density::opptical_depth.
	// Because everything happens in world space there could be differences
	// when the volume is rotated because the local aabb is never checked.

	const float ay = rn.origin[1] + rn.min_t * rn.direction[1];
	const float by = rn.origin[1] + rn.max_t * rn.direction[1];

	const float min_y = aabb_.min()[1];
	const float ha = ay - min_y;
	const float hb = by - min_y;

	const float3 attenuation = absorption_ + scattering_;

	const float d = rn.max_t - rn.min_t;

	const float hb_ha = hb - ha;

	if (0.f == hb_ha) {
		// special case where density stays exactly the same along the ray
		const float3 result = d * (a_ * math::exp(-b_ * ha)) * attenuation;

		SOFT_ASSERT(math::all_finite(result));

		return result;
	}

	// calculate the integral of the exponential density function
//	float fa = -((a_ * std::exp(-b_ * ha)) / b_);
//	float fb = -((a_ * std::exp(-b_ * hb)) / b_);

//	float3 result = d * ((fb - fa) / (hb - ha)) * attenuation;

	const float fa = -math::exp(-b_ * ha);
	const float fb = -math::exp(-b_ * hb);

	const float3 result = d * ((a_ * (fb - fa) / b_) / (hb_ha)) * attenuation;

	SOFT_ASSERT(math::all_finite(result));

	return result;

//	float3 old_result = Density::optical_depth(ray, step_size, rng, filter, worker);
//	return old_result;
}

float Height::density(const Transformation& transformation, const float3& p,
					  Sampler_filter /*filter*/, const Worker& /*worker*/) const {
	// p is in object space already

	// calculate height, relative to volume, in world space
	const float height = transformation.scale[1] * (1.f + p[1]);

	return a_ * math::exp(-b_ * height);
}

void Height::set_parameter(const std::string& name, const json::Value& value) {
	if ("a" == name) {
		a_ = json::read_float(value);
	} else if ("b" == name) {
		b_ = json::read_float(value);
	}
}

}
