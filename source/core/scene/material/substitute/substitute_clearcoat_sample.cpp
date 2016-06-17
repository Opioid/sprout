#include "substitute_clearcoat_sample.hpp"
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

	math::float3 c_reflection;
	float c_pdf;

	math::float3 c_fresnel = clearcoat_.weight * fresnel::schlick(wo_dot_h,
																  math::float3(clearcoat_.f0));

	if (0.f == clearcoat_.a2) {
		c_reflection = math::float3_identity;
		c_pdf = 0.f;
	} else {
		float c_clamped_a2 = ggx::clamp_a2(clearcoat_.a2);
		float c_d = ggx::distribution_isotropic(n_dot_h, c_clamped_a2);
		float c_g = ggx::geometric_visibility(n_dot_wi, n_dot_wo, c_clamped_a2);

		c_reflection = c_d * c_g * c_fresnel;
		c_pdf = c_d * n_dot_h / (4.f * wo_dot_h);
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

void Sample_clearcoat::sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const {
	if (!same_hemisphere(wo_)) {
		result.pdf = 0.f;
		return;
	}

	float p = sampler.generate_sample_1D();

	if (p < 0.5f) {
		float n_dot_wo = clamped_n_dot_wo();

		ggx::Isotropic specular;
		float n_dot_wi = specular.init_importance_sample(n_dot_wo, clearcoat_.a2, *this,
														 sampler, result);

		fresnel::Schlick_weighted clearcoat(clearcoat_.f0, clearcoat_.weight);
		math::float3 c_fresnel;
		float c_pdf;
		math::float3 c_reflection = specular.evaluate(n_dot_wi, n_dot_wo, clearcoat_.a2,
													   clearcoat, c_fresnel, c_pdf);

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
			pure_specular_importance_sample_and_clearcoat(sampler, result);
		} else {
			if (p < 0.75f) {
				diffuse_importance_sample_and_clearcoat(sampler, result);
			} else {
				specular_importance_sample_and_clearcoat(sampler, result);
			}
		}
	}
}

void Sample_clearcoat::set_clearcoat(const coating::Clearcoat& clearcoat) {
	clearcoat_ = clearcoat;
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

	fresnel::Schlick_weighted clearcoat(clearcoat_.f0, clearcoat_.weight);
	math::float3 c_fresnel;
	float c_pdf;
	math::float3 c_reflection = specular.evaluate(n_dot_wi, n_dot_wo, clearcoat_.a2,
												   clearcoat, c_fresnel, c_pdf);

	math::float3 base_layer = (1.f - c_fresnel) * (result.reflection + ggx_reflection);

	result.reflection = n_dot_wi * (c_reflection + base_layer);

	// PDF weight 0.5 * 0.5
	// 0.5 chance to select substitute layer and then 0.5 chance to select this importance sample
	result.pdf = 0.25f * (c_pdf + result.pdf + ggx_pdf);
}

void Sample_clearcoat::specular_importance_sample_and_clearcoat(sampler::Sampler& sampler,
																bxdf::Result& result) const {
	float n_dot_wo = clamped_n_dot_wo();

	ggx::Isotropic specular;
	float n_dot_wi = specular.init_importance_sample(n_dot_wo, a2_, *this, sampler, result);

	fresnel::Schlick_weighted clearcoat(clearcoat_.f0, clearcoat_.weight);
	math::float3 c_fresnel;
	float c_pdf;
	math::float3 c_reflection = specular.evaluate(n_dot_wi, n_dot_wo, clearcoat_.a2,
												  clearcoat, c_fresnel, c_pdf);

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

void Sample_clearcoat::pure_specular_importance_sample_and_clearcoat(sampler::Sampler& sampler,
																	 bxdf::Result& result) const {
	float n_dot_wo = clamped_n_dot_wo();

	ggx::Isotropic specular;
	float n_dot_wi = specular.init_importance_sample(n_dot_wo, a2_, *this, sampler, result);

	fresnel::Schlick_weighted clearcoat(clearcoat_.f0, clearcoat_.weight);
	math::float3 c_fresnel;
	float c_pdf;
	math::float3 c_reflection = specular.evaluate(n_dot_wi, n_dot_wo, clearcoat_.a2,
												  clearcoat, c_fresnel, c_pdf);

	fresnel::Schlick schlick(f0_);
	float ggx_pdf;
	math::float3 ggx_reflection = specular.evaluate(n_dot_wi, n_dot_wo, a2_, schlick, ggx_pdf);

	math::float3 base_layer = (1.f - c_fresnel) * ggx_reflection;

	result.reflection = n_dot_wi * (c_reflection + base_layer);
	result.pdf = 0.5f * (c_pdf + ggx_pdf);
}

}}}
