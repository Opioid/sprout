#include "display_sample.hpp"
#include "scene/material/material_sample.inl"
#include "sampler/sampler.hpp"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace display {

float3_p Sample::shading_normal() const {
	return layer_.n;
}

float3 Sample::tangent_to_world(float3_p v) const {
	return layer_.tangent_to_world(v);
}

float3 Sample::evaluate(float3_p wi, float& pdf) const {
	// Roughness zero will always have zero specular term (or worse NaN)
	if (0.f == layer_.a2) {
		pdf = 0.f;
		return math::float3_identity;
	}

	float n_dot_wi = layer_.clamped_n_dot(wi);
	float n_dot_wo = layer_.clamped_n_dot(wo_);

	float3 h = math::normalized(wo_ + wi);

	float n_dot_h  = math::dot(layer_.n, h);
	float wo_dot_h = math::dot(wo_, h);

	float clamped_a2 = ggx::clamp_a2(layer_.a2);
	float d = ggx::distribution_isotropic(n_dot_h, clamped_a2);
	float g = ggx::geometric_visibility(n_dot_wi, n_dot_wo, clamped_a2);
	float3 f = fresnel::schlick(wo_dot_h, layer_.f0);

	float3 specular = d * g * f;

	pdf = d * n_dot_h / (4.f * wo_dot_h);

	return n_dot_wi * specular;
}

float3 Sample::radiance() const {
	return layer_.emission;
}

float3 Sample::attenuation() const {
	return float3(100.f, 100.f, 100.f);
}

float Sample::ior() const {
	return 1.5f;
}

void Sample::sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const {
	if (!same_hemisphere(wo_)) {
		result.pdf = 0.f;
		return;
	}

	float n_dot_wo = layer_.clamped_n_dot(wo_);
	fresnel::Schlick schlick(layer_.f0);
	float n_dot_wi = ggx::Isotropic::importance_sample(wo_, n_dot_wo, layer_,
													   schlick, sampler, result);

	result.reflection = n_dot_wi * result.reflection;
}

bool Sample::is_pure_emissive() const {
	return false;
}

bool Sample::is_transmissive() const {
	return false;
}

bool Sample::is_translucent() const {
	return false;
}

void Sample::Layer::set(float3_p radiance, float f0, float roughness) {
	this->emission = radiance;
	this->f0 = float3(f0);
	this->a2 = math::pow4(roughness);
}

}}}
