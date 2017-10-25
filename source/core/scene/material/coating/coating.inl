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
float3 Clearcoat::evaluate(const float3& wi, const float3& wo, const float3& h,
						   float wo_dot_h, float /*internal_ior*/, const Layer& layer,
						   float3& attenuation, float& pdf) const {
	const float n_dot_wi = layer.clamp_n_dot(wi);
	const float n_dot_wo = layer.clamp_abs_n_dot(wo); //layer.clamp_n_dot(wo);

	const float n_dot_h = math::saturate(math::dot(layer.n_, h));

	const fresnel::Schlick_weighted schlick(f0_, weight_);
	const float3 result = n_dot_wi * ggx::Isotropic::reflection(n_dot_wi, n_dot_wo,
																wo_dot_h, n_dot_h,
																layer, schlick, attenuation, pdf);

	attenuation = (1.f - attenuation) * math::lerp(float3(1.f), color_, weight_);

	return result;
}

template<typename Layer>
void Clearcoat::sample(const float3& wo, float /*internal_ior*/,
					   const Layer& layer, sampler::Sampler& sampler,
					   float3& attenuation, bxdf::Result& result) const {
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
float3 Thinfilm::evaluate(const float3& wi, const float3& wo, const float3& h,
						  float wo_dot_h, float internal_ior, const Layer& layer,
						  float3& attenuation, float& pdf) const {
	const float n_dot_wi = layer.clamp_n_dot(wi);
	const float n_dot_wo = layer.clamp_abs_n_dot(wo); //layer.clamp_n_dot(wo);

	const float n_dot_h = math::saturate(math::dot(layer.n_, h));

	const fresnel::Thinfilm_weighted thinfilm(1.f, ior_, internal_ior, thickness_, weight_);
	const float3 result = n_dot_wi * ggx::Isotropic::reflection(n_dot_wi, n_dot_wo,
																wo_dot_h, n_dot_h,
																layer, thinfilm, attenuation, pdf);

	attenuation = (1.f - attenuation) * math::lerp(float3(1.f), color_, weight_);

	return result;
}

template<typename Layer>
void Thinfilm::sample(const float3& wo, float internal_ior, const Layer& layer,
					  sampler::Sampler& sampler, float3& attenuation, bxdf::Result& result) const {
	const float n_dot_wo = layer.clamp_abs_n_dot(wo); //layer.clamp_n_dot(wo);

	const fresnel::Thinfilm_weighted thinfilm(1.f, ior_, internal_ior, thickness_, weight_);

	const float n_dot_wi = ggx::Isotropic::reflect(wo, n_dot_wo, layer, thinfilm,
												   sampler, attenuation, result);

	attenuation = (1.f - attenuation) * math::lerp(float3(1.f), color_, weight_);

	result.reflection *= n_dot_wi;
}

template<typename Coating>
float3 Coating_layer<Coating>::evaluate(const float3& wi, const float3& wo, const float3& h,
										float wo_dot_h, float internal_ior,
										float3& attenuation, float& pdf) const {
	return Coating::evaluate(wi, wo, h, wo_dot_h, internal_ior, *this, attenuation, pdf);
}

template<typename Coating>
void Coating_layer<Coating>::sample(const float3& wo, float internal_ior,
									sampler::Sampler& sampler, float3& attenuation,
									bxdf::Result& result) const {
	Coating::sample(wo, internal_ior, *this, sampler, attenuation, result);
}

}
