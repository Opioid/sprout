#include "metallic_paint_sample.hpp"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/lambert/lambert.inl"
#include "scene/material/material_sample.inl"
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
//	float3 h = math::normalized(wo_ + wi);
//	float wo_dot_h = math::clamp(math::dot(wo_, h), 0.00001f, 1.f);


	float n_dot_wo = math::saturate(math::dot(layer_.n, wo_));


	float f0 = 0.02f;
	float f = fresnel::schlick(n_dot_wo, f0);

	float3 color = math::lerp(layer_.color_a, layer_.color_b, f);

	float n_dot_wi = layer_.clamped_n_dot(wi);

	return n_dot_wi * lambert::Isotropic::evaluate(color, n_dot_wi, layer_, pdf);
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
/*	float n_dot_wo = layer_.clamped_n_dot(wo_);

	fresnel::Conductor conductor(layer_.ior, layer_.absorption);
	float n_dot_wi = ggx::Isotropic::importance_sample(wo_, n_dot_wo, layer_,
													   conductor, sampler, result);
	result.reflection *= n_dot_wi;
	*/
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

void Sample::Layer::set(float3_p color_a, float3_p color_b) {
	this->color_a = color_a;
	this->color_b = color_b;
}

}}}
