#include "substitute_subsurface_sample.hpp"
#include "substitute_base_sample.inl"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"
#include "sampler/sampler.hpp"
#include "base/math/math.hpp"
#include "base/math/vector4.inl"
#include "base/math/sampling/sampling.inl"

namespace scene { namespace material { namespace substitute {

float3 Sample_subsurface::evaluate(float3_p wi, float& pdf) const {
	if (!same_hemisphere(wo_)) {
		pdf = 0.f;
		return float3::identity();
	}

	const float3 h = math::normalized(wo_ + wi);
	const float wo_dot_h = math::clamp(math::dot(wo_, h), 0.00001f, 1.f);

	return layer_.base_evaluate(wi, wo_, h, wo_dot_h, pdf);
}

void Sample_subsurface::sample(sampler::Sampler& sampler, bxdf::Result& result) const {
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

}}}

