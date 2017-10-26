#pragma once

#include "coating.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"

namespace scene::material::coating {

inline void Coating_base::set_color_and_weight(const float3& color, float weight) {
	color_ = color;
	weight_ = weight;
}

inline void Clearcoat::set(float f0, float alpha, float alpha2) {
	f0_ = f0;
	alpha_ = alpha;
	alpha2_ = alpha2;
}

template<typename Layer>
Result Clearcoat::evaluate(const float3& wi, const float3& wo, const float3& h,
						   float wo_dot_h, float /*internal_ior*/, const Layer& layer) const {
	const float n_dot_wi = layer.clamp_n_dot(wi);
	const float n_dot_wo = layer.clamp_abs_n_dot(wo); //layer.clamp_n_dot(wo);

	const float n_dot_h = math::saturate(math::dot(layer.n_, h));

	const fresnel::Schlick_weighted schlick(f0_, weight_);
	float3 fresnel;
	const auto ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h,
												layer, schlick, fresnel);

	const float3 attenuation = (1.f - fresnel) * math::lerp(float3(1.f), color_, weight_);

	return { n_dot_wi * ggx.reflection, attenuation, ggx.pdf };
}

template<typename Layer>
void Clearcoat::sample(const float3& wo, float /*internal_ior*/,
					   const Layer& layer, sampler::Sampler& sampler,
					   float3& attenuation, bxdf::Sample& result) const {
	const float n_dot_wo = layer.clamp_abs_n_dot(wo); //layer.clamp_n_dot(wo);

	const fresnel::Schlick_weighted schlick(f0_, weight_);

	const float n_dot_wi = ggx::Isotropic::reflect(wo, n_dot_wo, layer, schlick,
												   sampler, attenuation, result);

	attenuation = (1.f - attenuation) * math::lerp(float3(1.f), color_, weight_);

	result.reflection *= n_dot_wi;
}

inline void Thinfilm::set(float ior, float alpha, float alpha2, float thickness) {
	ior_ = ior;
	alpha_ = alpha;
	alpha2_  = alpha2;
	thickness_ = thickness;
}

template<typename Layer>
Result Thinfilm::evaluate(const float3& wi, const float3& wo, const float3& h,
						  float wo_dot_h, float internal_ior, const Layer& layer) const {
	const float n_dot_wi = layer.clamp_n_dot(wi);
	const float n_dot_wo = layer.clamp_abs_n_dot(wo); //layer.clamp_n_dot(wo);

	const float n_dot_h = math::saturate(math::dot(layer.n_, h));

	const fresnel::Thinfilm_weighted thinfilm({ 1.f, ior_, internal_ior, thickness_ }, weight_);
	float3 fresnel;
	const auto ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h,
												layer, thinfilm, fresnel);

	const float3 attenuation = (1.f - fresnel) * math::lerp(float3(1.f), color_, weight_);

	return { n_dot_wi * ggx.reflection, attenuation, ggx.pdf };
}

template<typename Layer>
void Thinfilm::sample(const float3& wo, float internal_ior, const Layer& layer,
					  sampler::Sampler& sampler, float3& attenuation, bxdf::Sample& result) const {
	const float n_dot_wo = layer.clamp_abs_n_dot(wo); //layer.clamp_n_dot(wo);

	const fresnel::Thinfilm_weighted thinfilm({ 1.f, ior_, internal_ior, thickness_ }, weight_);

	const float n_dot_wi = ggx::Isotropic::reflect(wo, n_dot_wo, layer, thinfilm,
												   sampler, attenuation, result);

	attenuation = (1.f - attenuation) * math::lerp(float3(1.f), color_, weight_);

	result.reflection *= n_dot_wi;
}

template<typename Coating>
Result Coating_layer<Coating>::evaluate(const float3& wi, const float3& wo, const float3& h,
										float wo_dot_h, float internal_ior) const {
	return Coating::evaluate(wi, wo, h, wo_dot_h, internal_ior, *this);
}

template<typename Coating>
void Coating_layer<Coating>::sample(const float3& wo, float internal_ior,
									sampler::Sampler& sampler, float3& attenuation,
									bxdf::Sample& result) const {
	Coating::sample(wo, internal_ior, *this, sampler, attenuation, result);
}

}
