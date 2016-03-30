#include "substitute_sample.hpp"
#include "rendering/integrator/surface/integrator_helper.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"
#include "sampler/sampler.hpp"
#include "base/math/vector.inl"
#include "base/math/math.hpp"
#include "base/math/sampling/sampling.inl"

namespace scene { namespace material { namespace substitute {

math::float3 Sample::evaluate(math::pfloat3 wi, float& pdf) const {
	if (!same_hemisphere(wo_)) {
		pdf = 0.f;
		return math::float3_identity;
	}

	return base_evaluate(wi, pdf);
}

void Sample::sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const {
	if (!same_hemisphere(wo_)) {
		result.pdf = 0.f;
		return;
	}

	if (1.f == metallic_) {
		pure_specular_importance_sample(sampler, result);
	} else {
		float p = sampler.generate_sample_1D();

		if (p < 0.5f) {
			diffuse_importance_sample(sampler, result);
		} else {
			specular_importance_sample(sampler, result);
		}
	}
}

void Sample::set(const math::float3& color, const math::float3& emission,
				 float constant_f0, float a2, float metallic) {
	diffuse_color_ = (1.f - metallic) * color;
	f0_ = math::lerp(math::float3(constant_f0), color, metallic);
	emission_ = emission;

	a2_ = a2;

	metallic_ = metallic;
}

}}}
