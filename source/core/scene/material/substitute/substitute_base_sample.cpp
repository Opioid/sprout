#include "substitute_base_sample.hpp"
#include "rendering/integrator/surface/integrator_helper.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/oren_nayar/oren_nayar.inl"
#include "sampler/sampler.hpp"
#include "base/math/vector.inl"
#include "base/math/math.hpp"
#include "base/math/sampling/sampling.inl"

namespace scene { namespace material { namespace substitute {

float3 Sample_base::radiance() const {
	return emission_;
}

float3 Sample_base::attenuation() const {
	return float3(100.f, 100.f, 100.f);
}

float Sample_base::ior() const {
	return ior_;
}

bool Sample_base::is_pure_emissive() const {
	return false;
}

bool Sample_base::is_transmissive() const {
	return false;
}

bool Sample_base::is_translucent() const {
	return false;
}

float3 Sample_base::base_evaluate(float3_p wi, float& pdf) const {
	float n_dot_wi = std::max(math::dot(n_, wi),  0.00001f);
	float n_dot_wo = std::max(math::dot(n_, wo_), 0.00001f);

	float diffuse_pdf;
	float3 diffuse = oren_nayar::Isotropic::evaluate(wi, n_dot_wi, n_dot_wo,
														   *this, diffuse_pdf);

	// Roughness zero will always have zero specular term (or worse NaN)
	if (0.f == a2_) {
		pdf = 0.5f * diffuse_pdf;
		return n_dot_wi * diffuse;
	}

	float3 h = math::normalized(wo_ + wi);

	float n_dot_h  = math::dot(n_, h);
	float wo_dot_h = math::dot(wo_, h);

	float clamped_a2 = ggx::clamp_a2(a2_);
	float d = ggx::distribution_isotropic(n_dot_h, clamped_a2);
	float g = ggx::geometric_visibility(n_dot_wi, n_dot_wo, clamped_a2);
	float3 f = fresnel::schlick(wo_dot_h, f0_);

	float3 specular = d * g * f;

	float ggx_pdf = d * n_dot_h / (4.f * wo_dot_h);

	pdf = 0.5f * (diffuse_pdf + ggx_pdf);

	return n_dot_wi * (diffuse + specular);
}

void Sample_base::diffuse_importance_sample(sampler::Sampler& sampler, bxdf::Result& result) const {
	float n_dot_wo = clamped_n_dot_wo();
	float n_dot_wi = oren_nayar::Isotropic::importance_sample(n_dot_wo, *this, sampler, result);

	fresnel::Schlick schlick(f0_);
	float ggx_pdf;
	float3 ggx_reflection = ggx::Isotropic::evaluate(result.wi, n_dot_wi, n_dot_wo,
														   *this, schlick, ggx_pdf);

	result.reflection = n_dot_wi * (result.reflection + ggx_reflection);
	result.pdf = 0.5f * (result.pdf + ggx_pdf);
}

void Sample_base::specular_importance_sample(sampler::Sampler& sampler,
											 bxdf::Result& result) const {
	float n_dot_wo = clamped_n_dot_wo();
	fresnel::Schlick schlick(f0_);
	float n_dot_wi = ggx::Isotropic::importance_sample(n_dot_wo, *this, schlick, sampler, result);

	float on_pdf;
	float3 on_reflection = oren_nayar::Isotropic::evaluate(
				result.wi, n_dot_wi, n_dot_wo, *this, on_pdf);

	result.reflection = n_dot_wi * (result.reflection + on_reflection);
	result.pdf = 0.5f * (result.pdf + on_pdf);
}

void Sample_base::pure_specular_importance_sample(sampler::Sampler& sampler,
												  bxdf::Result& result) const {
	float n_dot_wo = clamped_n_dot_wo();
	fresnel::Schlick schlick(f0_);
	float n_dot_wi = ggx::Isotropic::importance_sample(n_dot_wo, *this, schlick, sampler, result);
	result.reflection *= n_dot_wi;
}

void Sample_base::set(float3_p color, float3_p radiance,
					  float ior, float constant_f0, float a2, float metallic) {
	diffuse_color_ = (1.f - metallic) * color;
	f0_ = math::lerp(float3(constant_f0), color, metallic);
	emission_ = radiance;
	ior_ = ior;
	a2_ = a2;
	metallic_ = metallic;
}

}}}
