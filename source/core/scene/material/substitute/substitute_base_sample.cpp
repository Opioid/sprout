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

float3_p Sample_base::shading_normal() const {
	return layer_.n;
}

float3 Sample_base::tangent_to_world(float3_p v) const {
	return layer_.tangent_to_world(v);
}

float3 Sample_base::radiance() const {
	return layer_.emission;
}

float3 Sample_base::attenuation() const {
	return float3(100.f, 100.f, 100.f);
}

float Sample_base::ior() const {
	return layer_.ior;
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
	float n_dot_wi = layer_.clamped_n_dot(wi);
	float n_dot_wo = layer_.clamped_n_dot(wo_);

	float diffuse_pdf;
	float3 diffuse = oren_nayar::Isotropic::evaluate(wi, wo_, n_dot_wi, n_dot_wo,
													 layer_, diffuse_pdf);

	// Roughness zero will always have zero specular term (or worse NaN)
	if (0.f == layer_.a2) {
		pdf = 0.5f * diffuse_pdf;
		return n_dot_wi * diffuse;
	}

	float3 h = math::normalized(wo_ + wi);

	float n_dot_h  = math::dot(layer_.n, h);
	float wo_dot_h = math::dot(wo_, h);

	float clamped_a2 = ggx::clamp_a2(layer_.a2);
	float d = ggx::distribution_isotropic(n_dot_h, clamped_a2);
	float g = ggx::geometric_visibility(n_dot_wi, n_dot_wo, clamped_a2);
	float3 f = fresnel::schlick(wo_dot_h, layer_.f0);

	float3 specular = d * g * f;

	float ggx_pdf = d * n_dot_h / (4.f * wo_dot_h);

	pdf = 0.5f * (diffuse_pdf + ggx_pdf);

	return n_dot_wi * (diffuse + specular);
}

void Sample_base::Layer::set(float3_p color, float3_p radiance, float ior,
							 float constant_f0, float a2, float metallic) {
	this->diffuse_color = (1.f - metallic) * color;
	this->f0 = math::lerp(float3(constant_f0), color, metallic);
	this->emission = radiance;
	this->ior = ior;
	this->a2 = a2;
	this->metallic = metallic;
}

float3 Sample_base::Layer::base_evaluate(float3_p wi, float3_p wo, float& pdf) const {
	float n_dot_wi = clamped_n_dot(wi);
	float n_dot_wo = clamped_n_dot(wo);

	float diffuse_pdf;
	float3 diffuse = oren_nayar::Isotropic::evaluate(wi, wo, n_dot_wi, n_dot_wo,
													 *this, diffuse_pdf);

	// Roughness zero will always have zero specular term (or worse NaN)
	if (0.f == a2) {
		pdf = 0.5f * diffuse_pdf;
		return n_dot_wi * diffuse;
	}

	float3 h = math::normalized(wo + wi);

	float n_dot_h  = math::dot(n, h);
	float wo_dot_h = math::dot(wo, h);

	float clamped_a2 = ggx::clamp_a2(a2);
	float d = ggx::distribution_isotropic(n_dot_h, clamped_a2);
	float g = ggx::geometric_visibility(n_dot_wi, n_dot_wo, clamped_a2);
	float3 f = fresnel::schlick(wo_dot_h, f0);

	float3 specular = d * g * f;

	float ggx_pdf = d * n_dot_h / (4.f * wo_dot_h);

	pdf = 0.5f * (diffuse_pdf + ggx_pdf);

	return n_dot_wi * (diffuse + specular);
}

void Sample_base::Layer::diffuse_importance_sample(float3_p wo, sampler::Sampler& sampler,
												   bxdf::Result& result) const {
	float n_dot_wo = clamped_n_dot(wo);
	float n_dot_wi = oren_nayar::Isotropic::importance_sample(wo, n_dot_wo, *this,
															  sampler, result);

	fresnel::Schlick schlick(f0);
	float ggx_pdf;
	float3 ggx_reflection = ggx::Isotropic::evaluate(result.wi, wo, n_dot_wi, n_dot_wo,
													 *this, schlick, ggx_pdf);

	result.reflection = n_dot_wi * (result.reflection + ggx_reflection);
	result.pdf = 0.5f * (result.pdf + ggx_pdf);
}

void Sample_base::Layer::specular_importance_sample(float3_p wo, sampler::Sampler& sampler,
													bxdf::Result& result) const {
	float n_dot_wo = clamped_n_dot(wo);
	fresnel::Schlick schlick(f0);
	float n_dot_wi = ggx::Isotropic::importance_sample(wo, n_dot_wo, *this,
													   schlick, sampler, result);

	float on_pdf;
	float3 on_reflection = oren_nayar::Isotropic::evaluate(result.wi, wo, n_dot_wi, n_dot_wo,
														   *this, on_pdf);

	result.reflection = n_dot_wi * (result.reflection + on_reflection);
	result.pdf = 0.5f * (result.pdf + on_pdf);
}

void Sample_base::Layer::pure_specular_importance_sample(float3_p wo, sampler::Sampler& sampler,
														 bxdf::Result& result) const {
	float n_dot_wo = clamped_n_dot(wo);
	fresnel::Schlick schlick(f0);
	float n_dot_wi = ggx::Isotropic::importance_sample(wo, n_dot_wo, *this,
													   schlick, sampler, result);
	result.reflection *= n_dot_wi;
}

}}}
