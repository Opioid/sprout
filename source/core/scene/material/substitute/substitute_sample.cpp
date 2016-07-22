#include "substitute_sample.hpp"
#include "rendering/integrator/surface/integrator_helper.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"
#include "sampler/sampler.hpp"
#include "base/math/vector.inl"
#include "base/math/math.hpp"
#include "base/math/sampling/sampling.inl"

namespace scene { namespace material { namespace substitute {

float3 Sample::evaluate(float3_p wi, float& pdf) const {
	if (!same_hemisphere(wo_)) {
		pdf = 0.f;
		return math::float3_identity;
	}

	return layer_.base_evaluate(wi, wo_, pdf);
}

void Sample::sample(sampler::Sampler& sampler, bxdf::Result& result) const {
	if (!same_hemisphere(wo_)) {
		result.pdf = 0.f;
		return;
	}

	if (1.f == layer_.metallic) {
		layer_.pure_specular_importance_sample(wo_, sampler, result);
	} else {
		float p = sampler.generate_sample_1D();

		if (p < 0.5f) {
			layer_.diffuse_importance_sample(wo_, sampler, result);
		} else {
			layer_.specular_importance_sample(wo_, sampler, result);
		}
	}
}

}}}
