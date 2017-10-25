#pragma once

#include "substitute_base_sample.hpp"
#include "scene/material/material_sample_helper.hpp"
#include "scene/material/coating/coating.inl"
#include "scene/material/disney/disney.inl"
#include "base/math/vector3.inl"

#include <iostream>

namespace scene::material::substitute {

template<typename Diffuse>
const material::Sample::Layer& Sample_base<Diffuse>::base_layer() const {
	return layer_;
}

template<typename Diffuse>
float3 Sample_base<Diffuse>::radiance() const {
	return layer_.emission_;
}

template<typename Diffuse>
float Sample_base<Diffuse>::ior() const {
	return layer_.ior_;
}

template<typename Diffuse>
template<typename Coating>
bxdf::Result Sample_base<Diffuse>::base_and_coating_evaluate(const float3& wi,
															 const Coating& coating_layer) const {
	const float3 h = math::normalize(wo_ + wi);
	const float wo_dot_h = clamp_dot(wo_, h);

	float3 coating_attenuation;
	const auto coating = coating_layer.evaluate(wi, wo_, h, wo_dot_h, layer_.ior_,
												coating_attenuation);

	const auto base = layer_.base_evaluate(wi, wo_, h, wo_dot_h);

	const float pdf = (coating.pdf + 2.f * base.pdf) / 3.f;
	return { coating.reflection + coating_attenuation * base.reflection, pdf };
}

template<typename Diffuse>
template<typename Coating>
void Sample_base<Diffuse>::base_and_coating_sample(const Coating& coating_layer,
												   sampler::Sampler& sampler,
												   bxdf::Sample& result) const {
	const float p = sampler.generate_sample_1D();

	if (p < 0.5f) {
		float3 coating_attenuation;
		coating_layer.sample(wo_, layer_.ior_, sampler, coating_attenuation, result);

		const auto base = layer_.base_evaluate(result.wi, wo_, result.h, result.h_dot_wi);

		result.pdf = (result.pdf + 2.f * base.pdf) / 3.f;
		result.reflection = result.reflection + coating_attenuation * base.reflection;
	} else {
		if (1.f == layer_.metallic_) {
			pure_specular_sample_and_coating(coating_layer, sampler, result);
		} else {
			if (p < 0.75f) {
				diffuse_sample_and_coating(coating_layer, sampler, result);
			} else {
				specular_sample_and_coating(coating_layer, sampler, result);
			}
		}
	}
}

template<typename Diffuse>
template<typename Coating>
void Sample_base<Diffuse>::diffuse_sample_and_coating(const Coating& coating_layer,
													  sampler::Sampler& sampler,
													  bxdf::Sample& result) const {
	layer_.diffuse_sample(wo_, sampler, result);

	float3 coating_attenuation;
	const auto coating = coating_layer.evaluate(result.wi, wo_, result.h,
												result.h_dot_wi, layer_.ior_,
												coating_attenuation);

	result.pdf = (2.f * result.pdf + coating.pdf) / 3.f;
	result.reflection = coating_attenuation * result.reflection + coating.reflection;
}

template<typename Diffuse>
template<typename Coating>
void Sample_base<Diffuse>::specular_sample_and_coating(const Coating& coating_layer,
													   sampler::Sampler& sampler,
													   bxdf::Sample& result) const {
	layer_.specular_sample(wo_, sampler, result);

	float3 coating_attenuation;
	const auto coating = coating_layer.evaluate(result.wi, wo_, result.h,
												result.h_dot_wi, layer_.ior_,
												coating_attenuation);

	result.pdf = (2.f * result.pdf + coating.pdf) / 3.f;
	result.reflection = coating_attenuation * result.reflection + coating.reflection;
}

template<typename Diffuse>
template<typename Coating>
void Sample_base<Diffuse>::pure_specular_sample_and_coating(const Coating& coating_layer,
															sampler::Sampler& sampler,
															bxdf::Sample& result) const {
	layer_.pure_specular_sample(wo_, sampler, result);

	float3 coating_attenuation;
	const auto coating = coating_layer.evaluate(result.wi, wo_, result.h,
												result.h_dot_wi, layer_.ior_,
												coating_attenuation);

	result.pdf = 0.5f * (result.pdf + coating.pdf);
	result.reflection = coating_attenuation * result.reflection + coating.reflection;
}

template<typename Diffuse>
void Sample_base<Diffuse>::Layer::set(const float3& color, const float3& radiance, float ior,
									  float constant_f0, float roughness, float metallic) {
	diffuse_color_ = (1.f - metallic) * color;
	f0_ = math::lerp(float3(constant_f0), color, metallic);
	emission_ = radiance;
	ior_ = ior;
	roughness_ = roughness;
	const float alpha = roughness * roughness;
	alpha_ = alpha;
	alpha2_ = alpha * alpha;
	metallic_ = metallic;
}

template<typename Diffuse>
bxdf::Result Sample_base<Diffuse>::Layer::base_evaluate(const float3& wi, const float3& wo,
														const float3& h, float wo_dot_h) const {
	const float n_dot_wi = clamp_n_dot(wi);
	const float n_dot_wo = clamp_abs_n_dot(wo); //clamp_n_dot(wo);

	float d_pdf;
	const float3 d_reflection = Diffuse::reflection(wo_dot_h, n_dot_wi, n_dot_wo, *this, d_pdf);

	const float n_dot_h = math::saturate(math::dot(n_, h));

	const fresnel::Schlick schlick(f0_);
	float3 ggx_fresnel;
	float  ggx_pdf;
	const float3 ggx_reflection = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h,
															 *this, schlick, ggx_fresnel, ggx_pdf);

	const float pdf = 0.5f * (d_pdf + ggx_pdf);

	// Apparantly weight by (1 - fresnel) is not correct!
	// So here we assume Diffuse has the proper fresnel built in - which Disney does (?)

	return { n_dot_wi * (d_reflection + ggx_reflection), pdf };
}

template<typename Diffuse>
void Sample_base<Diffuse>::Layer::diffuse_sample(const float3& wo, sampler::Sampler& sampler,
												 bxdf::Sample& result) const {
	const float n_dot_wo = clamp_abs_n_dot(wo); //clamp_n_dot(wo);
	const float n_dot_wi = Diffuse::reflect(wo, n_dot_wo, *this, sampler, result);

	const float n_dot_h = math::saturate(math::dot(n_, result.h));

	const fresnel::Schlick schlick(f0_);
	float3 ggx_fresnel;
	float  ggx_pdf;
	const float3 ggx_reflection = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, result.h_dot_wi,
															 n_dot_h, *this, schlick,
															 ggx_fresnel, ggx_pdf);

	result.reflection = n_dot_wi * (result.reflection + ggx_reflection);
	result.pdf = 0.5f * (result.pdf + ggx_pdf);
}

template<typename Diffuse>
void Sample_base<Diffuse>::Layer::specular_sample(const float3& wo, sampler::Sampler& sampler,
												  bxdf::Sample& result) const {
	const float n_dot_wo = clamp_abs_n_dot(wo); //clamp_n_dot(wo);

	const fresnel::Schlick schlick(f0_);
	float3 ggx_fresnel;
	const float n_dot_wi = ggx::Isotropic::reflect(wo, n_dot_wo, *this, schlick,
												   sampler, ggx_fresnel, result);

	float d_pdf;
	const float3 d_reflection = Diffuse::reflection(result.h_dot_wi, n_dot_wi,
													n_dot_wo, *this, d_pdf);

	result.reflection = n_dot_wi * (result.reflection + d_reflection);
	result.pdf = 0.5f * (result.pdf + d_pdf);
}

template<typename Diffuse>
void Sample_base<Diffuse>::Layer::pure_specular_sample(const float3& wo, sampler::Sampler& sampler,
													   bxdf::Sample& result) const {
	const float n_dot_wo = clamp_abs_n_dot(wo); //clamp_n_dot(wo);

	const fresnel::Schlick schlick(f0_);
	const float n_dot_wi = ggx::Isotropic::reflect(wo, n_dot_wo, *this, schlick, sampler, result);
	result.reflection *= n_dot_wi;
}

template<typename Diffuse>
float Sample_base<Diffuse>::Layer::base_diffuse_fresnel_hack(float n_dot_wi, float n_dot_wo) const {
	// I think this is what we have to weigh lambert with if it is added to a microfacet BRDF.
	// At the moment this is only used with the "translucent" material,
	// which is kind of hacky anyway.

//	const float a = fresnel::schlick(n_dot_wi, f0_[0]);
//	const float b = fresnel::schlick(n_dot_wo, f0_[0]);
//	return std::max(a, b);

	// Same as above, but shorter
	return fresnel::schlick(std::min(n_dot_wi, n_dot_wo), f0_[0]);
}

}
