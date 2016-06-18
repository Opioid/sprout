#pragma once

#include "substitute_base_sample.hpp"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/oren_nayar/oren_nayar.inl"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace substitute {

template<typename Coating>
math::float3 Sample_base::base_evaluate_and_coating(math::pfloat3 wi, const Coating& coating,
													float coating_a2, float& pdf) const {
	float n_dot_wi = std::max(math::dot(n_, wi),  0.00001f);
	float n_dot_wo = std::max(math::dot(n_, wo_), 0.00001f);

	float diffuse_pdf;
	math::float3 diffuse = oren_nayar::Isotropic::evaluate(wi, n_dot_wi, n_dot_wo,
														   *this, diffuse_pdf);

	math::float3 h = math::normalized(wo_ + wi);

	float n_dot_h  = math::dot(n_, h);
	float wo_dot_h = math::dot(wo_, h);

	math::float3 c_reflection;
	float c_pdf;

	math::float3 c_fresnel = coating(wo_dot_h);
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
	if (0.f == a2_) {
		pdf = 0.5f * (c_pdf + 0.5f * diffuse_pdf);
		return n_dot_wi * (1.f - c_fresnel) * diffuse;
	}

	float clamped_a2 = ggx::clamp_a2(a2_);
	float d = ggx::distribution_isotropic(n_dot_h, clamped_a2);
	float g = ggx::geometric_visibility(n_dot_wi, n_dot_wo, clamped_a2);
	math::float3 f = fresnel::schlick(wo_dot_h, f0_);

	math::float3 specular = d * g * f;

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
		float n_dot_wo = clamped_n_dot_wo();

		ggx::Isotropic specular;
		float n_dot_wi = specular.init_importance_sample(n_dot_wo, coating_a2, *this,
														 sampler, result);

		math::float3 c_fresnel;
		float c_pdf;
		math::float3 c_reflection = specular.evaluate(n_dot_wi, n_dot_wo, coating_a2,
													  coating, c_fresnel, c_pdf);

		fresnel::Schlick schlick(f0_);
		float ggx_pdf;
		math::float3 ggx_reflection = specular.evaluate(n_dot_wi, n_dot_wo, a2_, schlick, ggx_pdf);

		float on_pdf;
		math::float3 on_reflection = oren_nayar::Isotropic::evaluate(
			result.wi, n_dot_wi, n_dot_wo, *this, on_pdf);

		math::float3 base_layer = (1.f - c_fresnel) * (on_reflection + ggx_reflection);

		result.reflection = n_dot_wi * (c_reflection + base_layer);
		result.pdf = 0.5f * (c_pdf + on_pdf + ggx_pdf);
	} else {
		if (1.f == metallic_) {
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
	float n_dot_wo = clamped_n_dot_wo();
	float n_dot_wi = oren_nayar::Isotropic::importance_sample(n_dot_wo, *this, sampler, result);

	ggx::Isotropic specular;
	specular.init_evaluate(result.wi, *this);

	fresnel::Schlick schlick(f0_);
	float ggx_pdf;
	math::float3 ggx_reflection = specular.evaluate(n_dot_wi, n_dot_wo, a2_, schlick, ggx_pdf);

	math::float3 c_fresnel;
	float c_pdf;
	math::float3 c_reflection = specular.evaluate(n_dot_wi, n_dot_wo, coating_a2,
												  coating, c_fresnel, c_pdf);

	math::float3 base_layer = (1.f - c_fresnel) * (result.reflection + ggx_reflection);

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
	float n_dot_wo = clamped_n_dot_wo();

	ggx::Isotropic specular;
	float n_dot_wi = specular.init_importance_sample(n_dot_wo, a2_, *this, sampler, result);

	math::float3 c_fresnel;
	float c_pdf;
	math::float3 c_reflection = specular.evaluate(n_dot_wi, n_dot_wo, coating_a2,
												  coating, c_fresnel, c_pdf);

	fresnel::Schlick schlick(f0_);
	float ggx_pdf;
	math::float3 ggx_reflection = specular.evaluate(n_dot_wi, n_dot_wo, a2_, schlick, ggx_pdf);

	float on_pdf;
	math::float3 on_reflection = oren_nayar::Isotropic::evaluate(result.wi, n_dot_wi, n_dot_wo,
																 *this, on_pdf);

	math::float3 base_layer = (1.f - c_fresnel) * (on_reflection + ggx_reflection);

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
	float n_dot_wo = clamped_n_dot_wo();

	ggx::Isotropic specular;
	float n_dot_wi = specular.init_importance_sample(n_dot_wo, a2_, *this, sampler, result);

	math::float3 c_fresnel;
	float c_pdf;
	math::float3 c_reflection = specular.evaluate(n_dot_wi, n_dot_wo, coating_a2,
												  coating, c_fresnel, c_pdf);

	fresnel::Schlick schlick(f0_);
	float ggx_pdf;
	math::float3 ggx_reflection = specular.evaluate(n_dot_wi, n_dot_wo, a2_, schlick, ggx_pdf);

	math::float3 base_layer = (1.f - c_fresnel) * ggx_reflection;

	result.reflection = n_dot_wi * (c_reflection + base_layer);
	result.pdf = 0.5f * (c_pdf + ggx_pdf);
}

}}}
