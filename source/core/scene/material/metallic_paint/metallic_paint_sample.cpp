#include "metallic_paint_sample.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/ggx/ggx.inl"
#include "sampler/sampler.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace metallic_paint {

float3_p Sample::shading_normal() const {
	return layer_.n;
}

float3 Sample::tangent_to_world(float3_p v) const {
	return layer_.tangent_to_world(v);
}

float3 Sample::evaluate(float3_p wi, float& pdf) const {
	float n_dot_wi = layer_.clamped_n_dot(wi);
	float n_dot_wo = layer_.clamped_n_dot(wo_);

	fresnel::Conductor conductor(layer_.ior, layer_.absorption);
	return n_dot_wi * ggx::Isotropic::evaluate(wi, wo_, n_dot_wi, n_dot_wo,
											   layer_, conductor, pdf);
}

float3 Sample::radiance() const {
	return math::float3_identity;
}

float3 Sample::attenuation() const {
	return float3(100.f, 100.f, 100.f);
}

float Sample::ior() const {
	return 1.5f;
}

void Sample::sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const {
	float n_dot_wo = layer_.clamped_n_dot(wo_);

	fresnel::Conductor conductor(layer_.ior, layer_.absorption);
	float n_dot_wi = ggx::Isotropic::importance_sample(wo_, n_dot_wo, layer_,
													   conductor, sampler, result);
	result.reflection *= n_dot_wi;
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

void Sample::Layer::set(float3_p ior, float3_p absorption, float roughness) {
	this->ior = ior;
	this->absorption = absorption;
	this->a2 = math::pow4(roughness);
}

}}}
