#include "substitute_sample_clearcoat.hpp"
#include "rendering/integrator/surface/integrator_helper.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/oren_nayar/oren_nayar.inl"
#include "sampler/sampler.hpp"
#include "base/math/vector.inl"
#include "base/math/math.hpp"
#include "base/math/sampling/sampling.inl"

namespace scene { namespace material { namespace substitute {

math::float3 Sample_clearcoat::evaluate(math::pfloat3 wi, float& pdf) const {
	if (!same_hemisphere(wo_)) {
		pdf = 0.f;
		return math::float3_identity;
	}

	float n_dot_wi = std::max(math::dot(n_, wi),  0.00001f);
	float n_dot_wo = std::max(math::dot(n_, wo_), 0.00001f);

	float diffuse_pdf;
	math::float3 diffuse = oren_nayar::Isotropic::evaluate(wi, n_dot_wi, n_dot_wo,
														   *this, diffuse_pdf);

	math::float3 h = math::normalized(wo_ + wi);

	float n_dot_h  = math::dot(n_, h);
	float wo_dot_h = math::dot(wo_, h);

	math::float3 cl_specular;
	float cl_pdf;

	math::float3 cl_f = fresnel::schlick(wo_dot_h, math::float3(clearcoat_f0_));
//	math::float3 cl_f = fresnel::thinfilm(wo_dot_h,
//										  1.f, clearcoat_ior_,
//										  ior_, clearcoat_thickness_);

	if (0.f == clearcoat_a2_) {
		cl_specular = math::float3_identity;
		cl_pdf = 0.f;
	} else {
		float cl_clamped_a2 = ggx::clamp_a2(clearcoat_a2_);
		float cl_d = ggx::distribution_isotropic(n_dot_h, cl_clamped_a2);
		float cl_g = ggx::geometric_shadowing(n_dot_wi, n_dot_wo, cl_clamped_a2);

		cl_specular = cl_d * cl_g * cl_f;
		cl_pdf = cl_d * n_dot_h / (4.f * wo_dot_h);
	}

	// Roughness zero will always have zero specular term (or worse NaN)
	if (0.f == a2_) {
		pdf = 0.5f * (cl_pdf + 0.5f * diffuse_pdf);
		return n_dot_wi * (1.f - cl_f) * diffuse;
	}

	float clamped_a2 = ggx::clamp_a2(a2_);
	float d = ggx::distribution_isotropic(n_dot_h, clamped_a2);
	float g = ggx::geometric_shadowing(n_dot_wi, n_dot_wo, clamped_a2);
	math::float3 f = fresnel::schlick(wo_dot_h, f0_);

	math::float3 specular = d * g * f;

	float ggx_pdf = d * n_dot_h / (4.f * wo_dot_h);

	pdf = 0.5f * (cl_pdf + 0.5f * (diffuse_pdf + ggx_pdf));

	return n_dot_wi * (cl_specular + (1.f - cl_f) * (diffuse + specular));
}

void Sample_clearcoat::sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const {
	if (!same_hemisphere(wo_)) {
		result.pdf = 0.f;
		return;
	}



	float n_dot_wo = clamped_n_dot_wo();

	ggx::Isotropic specular;
	float n_dot_wi = specular.init_importance_sample(n_dot_wo, clearcoat_a2_, *this, sampler,
													 result);

	float ior = 1.5f;

	// schlick

	float f0 = fresnel::schlick_f0(1.f, ior);
	fresnel::Schlick schlick(f0);
//	result.reflection = /*n_dot_wi **/ schlick(n_dot_wi);
//	result.pdf = 1.f;

	n_dot_wo = 0.0f;

	float schnuck = schlick(n_dot_wo).x;

	// dielectric

	float eta_i = 1.f / ior;
	float eta_t = ior;

	float sint2 = (eta_i * eta_i) * (1.f - n_dot_wo * n_dot_wo);

	float f;
	if (sint2 > 1.f) {
		f = 1.f;
	} else {
		float n_dot_t = std::sqrt(1.f - sint2);

		f = fresnel::dielectric(n_dot_wo, n_dot_t, eta_i, eta_t);
	}

	result.reflection = math::float3(f);
	result.pdf = 1.f;

	// thinfilm

//	float external_ior = 1.;
//	float thinfilm_ior = 1.8f;
//	float internal_ior = 1.47f;
//	float thickness = 400.f;

//	fresnel::Thinfilm thinfilm(external_ior, thinfilm_ior, internal_ior, thickness);
//	math::float3 cl_fresnel;
//	float cl_pdf;
//	math::float3 cl_reflection = specular.evaluate(n_dot_wi, n_dot_wo, clearcoat_a2_,
//												   thinfilm, cl_fresnel, cl_pdf);

//	result.reflection = n_dot_wi * cl_reflection;
//	result.pdf = cl_pdf;



/*
	float p = sampler.generate_sample_1D();

	if (p < 0.5f) {
		float n_dot_wo = clamped_n_dot_wo();

		ggx::Isotropic specular;
		float n_dot_wi = specular.init_importance_sample(n_dot_wo, clearcoat_a2_, *this,
														 sampler, result);
		fresnel::Schlick thinfilm((math::float3(clearcoat_f0_)));
//		fresnel::Thinfilm thinfilm(1.f, clearcoat_ior_, ior_, clearcoat_thickness_);
		math::float3 cl_fresnel;
		float cl_pdf;
		math::float3 cl_reflection = specular.evaluate(n_dot_wi, n_dot_wo, clearcoat_a2_,
													   thinfilm, cl_fresnel, cl_pdf);

		fresnel::Schlick schlick(f0_);
		float ggx_pdf;
		math::float3 ggx_reflection = specular.evaluate(n_dot_wi, n_dot_wo, a2_, schlick, ggx_pdf);

		float oren_nayar_pdf;
		math::float3 oren_nayar_reflection = oren_nayar::Isotropic::evaluate(
			result.wi, n_dot_wi, n_dot_wo, *this, oren_nayar_pdf);

		math::float3 base_layer = (1.f - cl_fresnel) * (oren_nayar_reflection + ggx_reflection);

		result.reflection = n_dot_wi * (cl_reflection + base_layer);
		result.pdf = 0.5f * (cl_pdf + oren_nayar_pdf + ggx_pdf);
	} else {
		if (1.f == metallic_) {
			pure_specular_importance_sample_and_clearcoat(sampler, result);
		} else {
			if (p < 0.75f) {
				diffuse_importance_sample_and_clearcoat(sampler, result);
			} else {
				specular_importance_sample_and_clearcoat(sampler, result);
			}
		}
	}
*/
}

void Sample_clearcoat::set(math::pfloat3 color, math::pfloat3 radiance,
						   float ior, float constant_f0, float a2, float metallic,
						   float clearcoat_ior, float clearcoat_a2, float clearcoat_thickess) {
	diffuse_color_ = (1.f - metallic) * color;
	f0_ = math::lerp(math::float3(constant_f0), color, metallic);
	emission_ = radiance;
	ior_ = ior;
	a2_ = a2;

	metallic_ = metallic;

	clearcoat_ior_ = clearcoat_ior;
	clearcoat_f0_ = fresnel::schlick_f0(1.f, clearcoat_ior);
	clearcoat_a2_ = clearcoat_a2;
	clearcoat_thickness_ = clearcoat_thickess;
}

void Sample_clearcoat::diffuse_importance_sample_and_clearcoat(sampler::Sampler& sampler,
															   bxdf::Result& result) const {
	float n_dot_wo = clamped_n_dot_wo();
	float n_dot_wi = oren_nayar::Isotropic::importance_sample(n_dot_wo, *this, sampler, result);

	ggx::Isotropic specular;
	specular.init_evaluate(result.wi, *this);

	fresnel::Schlick schlick(f0_);
	float ggx_pdf;
	math::float3 ggx_reflection = specular.evaluate(n_dot_wi, n_dot_wo, a2_, schlick, ggx_pdf);

	fresnel::Schlick thinfilm(clearcoat_f0_);
//	fresnel::Thinfilm thinfilm(1.f, clearcoat_ior_, ior_, clearcoat_thickness_);
	math::float3 cl_fresnel;
	float cl_pdf;
	math::float3 cl_reflection = specular.evaluate(n_dot_wi, n_dot_wo, clearcoat_a2_,
												   thinfilm, cl_fresnel, cl_pdf);

	math::float3 base_layer = (1.f - cl_fresnel) * (result.reflection + ggx_reflection);

	result.reflection = n_dot_wi * (cl_reflection + base_layer);

	// PDF weight 0.5 * 0.5
	// 0.5 chance to select substitute layer and then 0.5 chance to select this importance sample
	result.pdf = 0.25f * (cl_pdf + result.pdf + ggx_pdf);
}

void Sample_clearcoat::specular_importance_sample_and_clearcoat(sampler::Sampler& sampler,
																bxdf::Result& result) const {
	float n_dot_wo = clamped_n_dot_wo();

	ggx::Isotropic specular;
	float n_dot_wi = specular.init_importance_sample(n_dot_wo, a2_, *this, sampler, result);

	fresnel::Schlick thinfilm(clearcoat_f0_);
//	fresnel::Thinfilm thinfilm(1.f, clearcoat_ior_, ior_, clearcoat_thickness_);
	math::float3 cl_fresnel;
	float cl_pdf;
	math::float3 cl_reflection = specular.evaluate(n_dot_wi, n_dot_wo, clearcoat_a2_,
												   thinfilm, cl_fresnel, cl_pdf);

	fresnel::Schlick schlick(f0_);
	float ggx_pdf;
	math::float3 ggx_reflection = specular.evaluate(n_dot_wi, n_dot_wo, a2_, schlick, ggx_pdf);

	float oren_nayar_pdf;
	math::float3 oren_nayar_reflection = oren_nayar::Isotropic::evaluate(
		result.wi, n_dot_wi, n_dot_wo, *this, oren_nayar_pdf);

	math::float3 base_layer = (1.f - cl_fresnel) * (oren_nayar_reflection + ggx_reflection);

	result.reflection = n_dot_wi * (cl_reflection + base_layer);

	// PDF weight 0.5 * 0.5
	// 0.5 chance to select substitute layer and then 0.5 chance to select this importance sample
	result.pdf = 0.25f * (cl_pdf + oren_nayar_pdf + ggx_pdf);
}

void Sample_clearcoat::pure_specular_importance_sample_and_clearcoat(sampler::Sampler& sampler,
																	 bxdf::Result& result) const {
	float n_dot_wo = clamped_n_dot_wo();

	ggx::Isotropic specular;
	float n_dot_wi = specular.init_importance_sample(n_dot_wo, a2_, *this, sampler, result);

	fresnel::Thinfilm thinfilm(1.f, clearcoat_ior_, ior_, clearcoat_thickness_);
	math::float3 cl_fresnel;
	float cl_pdf;
	math::float3 cl_reflection = specular.evaluate(n_dot_wi, n_dot_wo, clearcoat_a2_,
												   thinfilm, cl_fresnel, cl_pdf);

	fresnel::Schlick schlick(f0_);
	float ggx_pdf;
	math::float3 ggx_reflection = specular.evaluate(n_dot_wi, n_dot_wo, a2_, schlick, ggx_pdf);

	math::float3 base_layer = (1.f - cl_fresnel) * ggx_reflection;

	result.reflection = n_dot_wi * (cl_reflection + base_layer);
	result.pdf = 0.5f * (cl_pdf + ggx_pdf);
}

}}}
