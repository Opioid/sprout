#pragma once

#include "substitute_base_sample.hpp"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/oren_nayar/oren_nayar.inl"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace substitute {

template<typename Coating>
float3 Sample_base::base_evaluate_and_coating(float3_p wi, const Coating& coating,
											  float coating_a2, float& pdf) const {
	float n_dot_wi = layer_.clamped_n_dot(wi);
	float n_dot_wo = layer_.clamped_n_dot(wo_);

	float diffuse_pdf;
	float3 diffuse = oren_nayar::Isotropic::evaluate(wi, n_dot_wi, n_dot_wo,
													 *this, layer_, diffuse_pdf);

	float3 h = math::normalized(wo_ + wi);

	float n_dot_h  = math::dot(layer_.n, h);
	float wo_dot_h = math::dot(wo_, h);

	float3 c_reflection;
	float c_pdf;

	float3 c_fresnel = coating(wo_dot_h);
	if (0.f == coating_a2) {
		c_reflection = math::float3_identity;
		c_pdf = 0.f;
	} else {
		float cl_clamped_a2 = ggx::clamp_a2(coating_a2);
		float cl_d = ggx::distribution_isotropic(n_dot_h, cl_clamped_a2);
		float cl_g = ggx::geometric_visibility(n_dot_wi, n_dot_wo, cl_clamped_a2);

		c_reflection = cl_d * cl_g * c_fresnel;
		c_pdf = cl_d * n_dot_h / (4.f * wo_dot_h);
	}

	// Roughness zero will always have zero specular term (or worse NaN)
	if (0.f == layer_.a2) {
		pdf = 0.5f * (c_pdf + 0.5f * diffuse_pdf);
		return n_dot_wi * (1.f - c_fresnel) * diffuse;
	}

	float clamped_a2 = ggx::clamp_a2(layer_.a2);
	float d = ggx::distribution_isotropic(n_dot_h, clamped_a2);
	float g = ggx::geometric_visibility(n_dot_wi, n_dot_wo, clamped_a2);
	float3 f = fresnel::schlick(wo_dot_h, layer_.f0);

	float3 specular = d * g * f;

	float ggx_pdf = d * n_dot_h / (4.f * wo_dot_h);

	pdf = 0.5f * (c_pdf + 0.5f * (diffuse_pdf + ggx_pdf));

	return n_dot_wi * (c_reflection + (1.f - c_fresnel) * (diffuse + specular));
}

template<typename Coating>
void Sample_base::base_sample_evaluate_and_coating(const Coating& coating,
												   float coating_a2,
												   sampler::Sampler& sampler,
												   bxdf::Result& result) const {
	float p = sampler.generate_sample_1D();

	if (p < 0.5f) {
		float n_dot_wo = layer_.clamped_n_dot(wo_);

		ggx::Isotropic specular;
		float n_dot_wi = specular.init_importance_sample(n_dot_wo, coating_a2, *this, layer_,
														 sampler, result);

		float3 c_fresnel;
		float c_pdf;
		float3 c_reflection = specular.evaluate(n_dot_wi, n_dot_wo, coating_a2,
												coating, c_fresnel, c_pdf);

		fresnel::Schlick schlick(layer_.f0);
		float ggx_pdf;
		float3 ggx_reflection = specular.evaluate(n_dot_wi, n_dot_wo, layer_.a2, schlick, ggx_pdf);

		float on_pdf;
		float3 on_reflection = oren_nayar::Isotropic::evaluate(result.wi, n_dot_wi, n_dot_wo,
															   *this, layer_, on_pdf);

		float3 base_layer = (1.f - c_fresnel) * (on_reflection + ggx_reflection);

		result.reflection = n_dot_wi * (c_reflection + base_layer);
		result.pdf = 0.5f * (c_pdf + on_pdf + ggx_pdf);
	} else {
		if (1.f == layer_.metallic) {
			pure_specular_importance_sample_and_coating(coating, coating_a2, sampler, result);
		} else {
			if (p < 0.75f) {
				diffuse_importance_sample_and_coating(coating, coating_a2, sampler, result);
			} else {
				specular_importance_sample_and_coating(coating, coating_a2, sampler, result);
			}
		}
	}
}

template<typename Coating>
void Sample_base::diffuse_importance_sample_and_coating(const Coating& coating,
														float coating_a2,
														sampler::Sampler& sampler,
														bxdf::Result& result) const {
	float n_dot_wo = layer_.clamped_n_dot(wo_);
	float n_dot_wi = oren_nayar::Isotropic::importance_sample(n_dot_wo, *this, layer_,
															  sampler, result);

	ggx::Isotropic specular;
	specular.init_evaluate(result.wi, *this, layer_);

	fresnel::Schlick schlick(layer_.f0);
	float ggx_pdf;
	float3 ggx_reflection = specular.evaluate(n_dot_wi, n_dot_wo, layer_.a2, schlick, ggx_pdf);

	float3 c_fresnel;
	float c_pdf;
	float3 c_reflection = specular.evaluate(n_dot_wi, n_dot_wo, coating_a2,
											coating, c_fresnel, c_pdf);

	float3 base_layer = (1.f - c_fresnel) * (result.reflection + ggx_reflection);

	result.reflection = n_dot_wi * (c_reflection + base_layer);

	// PDF weight 0.5 * 0.5
	// 0.5 chance to select substitute layer and then 0.5 chance to select this importance sample
	result.pdf = 0.25f * (c_pdf + result.pdf + ggx_pdf);
}

template<typename Coating>
void Sample_base::specular_importance_sample_and_coating(const Coating& coating,
														 float coating_a2,
														 sampler::Sampler& sampler,
														 bxdf::Result& result) const {
	float n_dot_wo = layer_.clamped_n_dot(wo_);

	ggx::Isotropic specular;
	float n_dot_wi = specular.init_importance_sample(n_dot_wo, layer_.a2, *this, layer_,
													 sampler, result);

	float3 c_fresnel;
	float c_pdf;
	float3 c_reflection = specular.evaluate(n_dot_wi, n_dot_wo, coating_a2,
											coating, c_fresnel, c_pdf);

	fresnel::Schlick schlick(layer_.f0);
	float ggx_pdf;
	float3 ggx_reflection = specular.evaluate(n_dot_wi, n_dot_wo, layer_.a2, schlick, ggx_pdf);

	float on_pdf;
	float3 on_reflection = oren_nayar::Isotropic::evaluate(result.wi, n_dot_wi, n_dot_wo,
														   *this, layer_, on_pdf);

	float3 base_layer = (1.f - c_fresnel) * (on_reflection + ggx_reflection);

	result.reflection = n_dot_wi * (c_reflection + base_layer);

	// PDF weight 0.5 * 0.5
	// 0.5 chance to select substitute layer and then 0.5 chance to select this importance sample
	result.pdf = 0.25f * (c_pdf + on_pdf + ggx_pdf);
}


template<typename Coating>
void Sample_base::pure_specular_importance_sample_and_coating(const Coating& coating,
															  float coating_a2,
															  sampler::Sampler& sampler,
															  bxdf::Result& result) const {
	float n_dot_wo = layer_.clamped_n_dot(wo_);

	ggx::Isotropic specular;
	float n_dot_wi = specular.init_importance_sample(n_dot_wo, layer_.a2, *this, layer_,
													 sampler, result);

	float3 c_fresnel;
	float c_pdf;
	float3 c_reflection = specular.evaluate(n_dot_wi, n_dot_wo, coating_a2,
												  coating, c_fresnel, c_pdf);

	fresnel::Schlick schlick(layer_.f0);
	float ggx_pdf;
	float3 ggx_reflection = specular.evaluate(n_dot_wi, n_dot_wo, layer_.a2, schlick, ggx_pdf);

	float3 base_layer = (1.f - c_fresnel) * ggx_reflection;

	result.reflection = n_dot_wi * (c_reflection + base_layer);
	result.pdf = 0.5f * (c_pdf + ggx_pdf);
}

}}}
