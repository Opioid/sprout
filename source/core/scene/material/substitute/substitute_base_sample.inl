#pragma once

#include "substitute_base_sample.hpp"
#include "scene/material/coating/coating.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/oren_nayar/oren_nayar.inl"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace substitute {

template<typename Coating>
float3 Sample_base::base_evaluate_and_coating(float3_p wi, const Coating& coating,
											  float& pdf) const {
	float n_dot_wi = layer_.clamped_n_dot(wi);
	float n_dot_wo = layer_.clamped_n_dot(wo_);

	float diffuse_pdf;
	float3 diffuse = oren_nayar::Isotropic::evaluate(wi, wo_, n_dot_wi, n_dot_wo,
													 layer_, diffuse_pdf);

	float3 coating_fresnel;
	float  coating_pdf;
	float3 coating_reflection = coating.evaluate(wi, wo_, layer_.ior,
												 coating_fresnel, coating_pdf);

	// Roughness zero will always have zero specular term (or worse NaN)
	if (0.f == layer_.a2) {
		pdf = 0.5f * (coating_pdf + 0.5f * diffuse_pdf);
		return coating_reflection + n_dot_wi * (1.f - coating_fresnel) * diffuse;
	}

	float3 h = math::normalized(wo_ + wi);

	float wo_dot_h = math::clamp(math::dot(wo_, h), 0.00001f, 1.f);
	float n_dot_h  = math::saturate(math::dot(layer_.n, h));

	float clamped_a2 = ggx::clamp_a2(layer_.a2);
	float d = ggx::distribution_isotropic(n_dot_h, clamped_a2);
	float g = ggx::geometric_visibility(n_dot_wi, n_dot_wo, clamped_a2);
	float3 f = fresnel::schlick(wo_dot_h, layer_.f0);

	float3 specular = d * g * f;

	float ggx_pdf = d * n_dot_h / (4.f * wo_dot_h);

	pdf = 0.5f * (coating_pdf + 0.5f * (diffuse_pdf + ggx_pdf));

	return coating_reflection + n_dot_wi * (1.f - coating_fresnel) * (diffuse + specular);
}

template<typename Coating>
void Sample_base::base_sample_evaluate_and_coating(const Coating& coating,
												   sampler::Sampler& sampler,
												   bxdf::Result& result) const {
/*	float p = sampler.generate_sample_1D();

	if (p < 0.5f) {
		float3 fresnel_c;
		coating.importance_sample(wo_, layer_.ior, sampler, fresnel_c, result);

		float base_pdf;
		float3 base_reflection = layer_.base_evaluate(result.wi, wo_, base_pdf);

		result.pdf = result.pdf;// + base_pdf;
		result.reflection = result.reflection + (1.f - fresnel_c) * base_reflection;
	} else {
		if (1.f == layer_.metallic) {
			pure_specular_importance_sample_and_coating(coating, sampler, result);
		} else {
			if (p < 0.75f) {
				diffuse_importance_sample_and_coating(coating, sampler, result);
			} else {
				specular_importance_sample_and_coating(coating, sampler, result);
			}
		}
	}
	*/

//	diffuse_importance_sample_and_coating(coating, sampler, result);

//	specular_importance_sample_and_coating(coating, sampler, result);


	float p = sampler.generate_sample_1D();

	if (p < 0.5f) {
		float3 fresnel_c;
		coating.importance_sample(wo_, layer_.ior, sampler, fresnel_c, result);


		result.pdf *= 0.5f;

	} else {
		if (1.f == layer_.metallic) {
		//	pure_specular_importance_sample_and_coating(coating, sampler, result);
		} else {
			if (p < 0.75f) {
				diffuse_importance_sample_and_coating(coating, sampler, result);
			} else {
				specular_importance_sample_and_coating(coating, sampler, result);
			}

			result.pdf *= 0.5f;
		}
	}
}

template<typename Coating>
void Sample_base::diffuse_importance_sample_and_coating(const Coating& coating,
														sampler::Sampler& sampler,
														bxdf::Result& result) const {
	layer_.diffuse_importance_sample(wo_, sampler, result);

	float3 coating_fresnel;
	float  coating_pdf;
	float3 coating_reflection = coating.evaluate(result.wi, wo_, layer_.ior,
												 coating_fresnel, coating_pdf);

//	result.pdf = result.pdf + 0.5f * coating_pdf;
//	result.pdf = (2.f * result.pdf + coating_pdf) / 3.f;
	result.reflection = (1.f - coating_fresnel) * result.reflection;// + coating_reflection;
}

template<typename Coating>
void Sample_base::specular_importance_sample_and_coating(const Coating& coating,
														 sampler::Sampler& sampler,
														 bxdf::Result& result) const {
	layer_.specular_importance_sample(wo_, sampler, result);

	float3 coating_fresnel;
	float  coating_pdf;
	float3 coating_reflection = coating.evaluate(result.wi, wo_, layer_.ior,
												 coating_fresnel, coating_pdf);

//	result.pdf = 0.5f * (result.pdf + coating_pdf);
	result.reflection = (1.f - coating_fresnel) * result.reflection; //+ coating_reflection;
}

template<typename Coating>
void Sample_base::pure_specular_importance_sample_and_coating(const Coating& coating,
															  sampler::Sampler& sampler,
															  bxdf::Result& result) const {
	layer_.pure_specular_importance_sample(wo_, sampler, result);

	float3 coating_fresnel;
	float  coating_pdf;
	float3 coating_reflection = coating.evaluate(result.wi, wo_, layer_.ior,
												 coating_fresnel, coating_pdf);

	result.pdf = 0.5f * (result.pdf + coating_pdf);
	result.reflection = (1.f - coating_fresnel) * result.reflection + coating_reflection;
}

}}}
