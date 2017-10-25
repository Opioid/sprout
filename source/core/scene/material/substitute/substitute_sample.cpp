#include "substitute_sample.hpp"
#include "substitute_base_sample.inl"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"
#include "sampler/sampler.hpp"
#include "base/math/math.hpp"
#include "base/math/vector4.inl"
#include "base/math/sampling/sampling.hpp"

namespace scene::material::substitute {

bxdf::Result Sample::evaluate(const float3& wi) const {
	if (!same_hemisphere(wo_)) {
		return { float3::identity(), 0.f };
	}

	const float3 h = math::normalize(wo_ + wi);
	const float wo_dot_h = clamp_dot(wo_, h);

	return layer_.base_evaluate(wi, wo_, h, wo_dot_h);
}

void Sample::sample(sampler::Sampler& sampler, bxdf::Sample& result) const {
	if (!same_hemisphere(wo_)) {
		result.pdf = 0.f;
		return;
	}

	if (1.f == layer_.metallic_) {
		layer_.pure_specular_sample(wo_, sampler, result);
	} else {
		const float p = sampler.generate_sample_1D();

		if (p < 0.5f) {
			layer_.diffuse_sample(wo_, sampler, result);
		} else {
			layer_.specular_sample(wo_, sampler, result);
		}
	}
}

}
