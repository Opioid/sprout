#pragma once

#include "substitute_base_sample.hpp"
#include "scene/material/coating/coating.inl"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace substitute {

template<typename Coating>
float3 Sample_base::base_evaluate_and_coating(float3_p wi, const Coating& coating,
											  float& pdf) const {
	float3 h = math::normalized(wo_ + wi);
	float wo_dot_h = math::clamp(math::dot(wo_, h), 0.00001f, 1.f);

	float3 coating_attenuation;
	float  coating_pdf;
	float3 coating_reflection = coating.evaluate(wi, wo_, h, wo_dot_h, layer_.ior_,
												 coating_attenuation, coating_pdf);

	float base_pdf;
	float3 base_reflection = layer_.base_evaluate(wi, wo_, h, wo_dot_h, base_pdf);

	pdf = (coating_pdf + 2.f * base_pdf) / 3.f;
	return coating_reflection + coating_attenuation * base_reflection;
}

template<typename Coating>
void Sample_base::base_sample_and_coating(const Coating& coating,
										  sampler::Sampler& sampler,
										  bxdf::Result& result) const {
	float p = sampler.generate_sample_1D();

	if (p < 0.5f) {
		float3 coating_attenuation;
		coating.sample(wo_, layer_.ior_, sampler, coating_attenuation, result);

		float base_pdf;
		float3 base_reflection = layer_.base_evaluate(result.wi, wo_, result.h,
													  result.h_dot_wi, base_pdf);

		result.pdf = (result.pdf + 2.f * base_pdf) / 3.f;
		result.reflection = result.reflection + coating_attenuation * base_reflection;
	} else {
		if (1.f == layer_.metallic_) {
			pure_specular_sample_and_coating(coating, sampler, result);
		} else {
			if (p < 0.75f) {
				diffuse_sample_and_coating(coating, sampler, result);
			} else {
				specular_sample_and_coating(coating, sampler, result);
			}
		}
	}
}

template<typename Coating>
void Sample_base::diffuse_sample_and_coating(const Coating& coating,
											 sampler::Sampler& sampler,
											 bxdf::Result& result) const {
	layer_.diffuse_sample(wo_, sampler, result);

	float3 coating_attenuation;
	float  coating_pdf;
	float3 coating_reflection = coating.evaluate(result.wi, wo_, result.h, result.h_dot_wi,
												 layer_.ior_, coating_attenuation, coating_pdf);

	result.pdf = (2.f * result.pdf + coating_pdf) / 3.f;
	result.reflection = coating_attenuation * result.reflection + coating_reflection;
}

template<typename Coating>
void Sample_base::specular_sample_and_coating(const Coating& coating,
											  sampler::Sampler& sampler,
											  bxdf::Result& result) const {
	layer_.specular_sample(wo_, sampler, result);

	float3 coating_attenuation;
	float  coating_pdf;
	float3 coating_reflection = coating.evaluate(result.wi, wo_, result.h, result.h_dot_wi,
												 layer_.ior_, coating_attenuation, coating_pdf);

	result.pdf = (2.f * result.pdf + coating_pdf) / 3.f;
	result.reflection = coating_attenuation * result.reflection + coating_reflection;
}

template<typename Coating>
void Sample_base::pure_specular_sample_and_coating(const Coating& coating,
												   sampler::Sampler& sampler,
												   bxdf::Result& result) const {
	layer_.pure_specular_sample(wo_, sampler, result);

	float3 coating_attenuation;
	float  coating_pdf;
	float3 coating_reflection = coating.evaluate(result.wi, wo_, result.h, result.h_dot_wi,
												 layer_.ior_, coating_attenuation, coating_pdf);

	result.pdf = 0.5f * (result.pdf + coating_pdf);
	result.reflection = coating_attenuation * result.reflection + coating_reflection;
}

}}}
