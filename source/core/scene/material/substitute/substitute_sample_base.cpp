#include "substitute_sample_base.hpp"
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

math::float3 Sample_base::emission() const {
	return emission_;
}

math::float3 Sample_base::attenuation() const {
	return math::float3(100.f, 100.f, 100.f);
}

float Sample_base::ior() const {
	return 0.f;
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

math::float3 Sample_base::base_evaluate(math::pfloat3 wi, float& pdf) const {
	float n_dot_wi = std::max(math::dot(n_, wi),  0.00001f);
	float n_dot_wo = std::max(math::dot(n_, wo_), 0.00001f);

	float diffuse_pdf;
	math::float3 diffuse = oren_nayar::Oren_nayar::evaluate(*this, wi, n_dot_wi, n_dot_wo, diffuse_pdf);

	// Roughness zero will always have zero specular term (or worse NaN)
	if (0.f == a2_) {
		pdf = 0.5f * diffuse_pdf;
		return n_dot_wi * diffuse;
	}

	math::float3 h = math::normalized(wo_ + wi);

	float n_dot_h  = math::dot(n_, h);
	float wo_dot_h = math::dot(wo_, h);

	float clamped_a2 = ggx::clamp_a2(a2_);
	float d = ggx::distribution_isotropic(n_dot_h, clamped_a2);
	float g = ggx::geometric_shadowing(n_dot_wi, n_dot_wo, clamped_a2);
	math::float3 f = fresnel::schlick(wo_dot_h, f0_);

	math::float3 specular = d * g * f;

	float ggx_pdf = d * n_dot_h / (4.f * wo_dot_h);

	pdf = 0.5f * (diffuse_pdf + ggx_pdf);

	return n_dot_wi * (diffuse + specular);
}

void Sample_base::diffuse_importance_sample(sampler::Sampler& sampler, bxdf::Result& result) const {
	float n_dot_wo = clamped_n_dot_wo();
	float n_dot_wi = oren_nayar::Oren_nayar::importance_sample(*this, sampler, n_dot_wo, result);

	float ggx_pdf;
	math::float3 ggx_reflection = ggx::Schlick_isotropic::evaluate(*this, result.wi, n_dot_wi, n_dot_wo, ggx_pdf);

	result.reflection = n_dot_wi * (result.reflection + ggx_reflection);
	result.pdf = 0.5f * (result.pdf + ggx_pdf);
}

void Sample_base::specular_importance_sample(sampler::Sampler& sampler, bxdf::Result& result) const {
	float n_dot_wo = clamped_n_dot_wo();
	float n_dot_wi = ggx::Schlick_isotropic::importance_sample(*this, sampler, n_dot_wo, result);

	float oren_nayar_pdf;
	math::float3 oren_nayar_reflection = oren_nayar::Oren_nayar::evaluate(
				*this, result.wi, n_dot_wi, n_dot_wo, oren_nayar_pdf);

	result.reflection = n_dot_wi * (result.reflection + oren_nayar_reflection);
	result.pdf = 0.5f * (result.pdf + oren_nayar_pdf);
}

void Sample_base::pure_specular_importance_sample(sampler::Sampler& sampler, bxdf::Result& result) const {
	float n_dot_wo = clamped_n_dot_wo();
	float n_dot_wi = ggx::Schlick_isotropic::importance_sample(*this, sampler, n_dot_wo, result);
	result.reflection *= n_dot_wi;
}

}}}
