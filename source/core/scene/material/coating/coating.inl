#pragma once

#include "coating.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"

namespace scene { namespace material { namespace coating {

inline void Clearcoat::set(float f0, float a2, float weight) {
	this->f0 = f0;
	this->a2 = a2;
	this->weight = weight;
}

template<typename Layer>
float3 Clearcoat::evaluate(float3_p wi, float3_p wo, float /*internal_ior*/, const Layer& layer,
						   float3& fresnel_result, float& pdf) const {
	float n_dot_wi = layer.clamped_n_dot(wi);
	float n_dot_wo = layer.clamped_n_dot(wo);

	fresnel::Schlick_weighted schlick(f0, weight);

	return n_dot_wi * ggx::Isotropic::evaluate(wi, wo, n_dot_wi, n_dot_wo, layer,
											   schlick, fresnel_result, pdf);
}

template<typename Layer>
void Clearcoat::importance_sample(float3_p wo, float /*internal_ior*/,
								  const Layer& layer, sampler::Sampler& sampler,
								  float3& fresnel_result, bxdf::Result& result) const {
	float n_dot_wo = layer.clamped_n_dot(wo);

	fresnel::Schlick_weighted schlick(f0, weight);

	float n_dot_wi = ggx::Isotropic::importance_sample(wo, n_dot_wo, layer,
													   schlick, sampler,
													   fresnel_result, result);
	result.reflection *= n_dot_wi;
}

inline void Thinfilm::set(float ior, float a2, float thickness, float weight) {
	this->ior = ior;
	this->a2  = a2;
	this->thickness = thickness;
	this->weight = weight;
}

template<typename Layer>
float3 Thinfilm::evaluate(float3_p wi, float3_p wo, float internal_ior, const Layer& layer,
						  float3& fresnel_result, float& pdf) const {
	float n_dot_wi = layer.clamped_n_dot(wi);
	float n_dot_wo = layer.clamped_n_dot(wo);

	fresnel::Thinfilm_weighted thinfilm(1.f, ior, internal_ior, thickness, weight);

	return n_dot_wi * ggx::Isotropic::evaluate(wi, wo, n_dot_wi, n_dot_wo, layer,
											   thinfilm, fresnel_result, pdf);
}

template<typename Layer>
void Thinfilm::importance_sample(float3_p wo, float internal_ior,
								 const Layer& layer, sampler::Sampler& sampler,
								 float3& fresnel_result, bxdf::Result& result) const {
	float n_dot_wo = layer.clamped_n_dot(wo);

	fresnel::Thinfilm_weighted thinfilm(1.f, ior, internal_ior, thickness, weight);

	float n_dot_wi = ggx::Isotropic::importance_sample(wo, n_dot_wo, layer,
													   thinfilm, sampler,
													   fresnel_result, result);
	result.reflection *= n_dot_wi;
}

template<typename Coating>
float3 Coating_layer<Coating>::evaluate(float3_p wi, float3_p wo, float internal_ior,
										float3& fresnel_result, float& pdf) const {
	return Coating::evaluate(wi, wo, internal_ior, *this, fresnel_result, pdf);
}

template<typename Coating>
void Coating_layer<Coating>::importance_sample(float3_p wo, float internal_ior,
											   sampler::Sampler& sampler, float3& fresnel_result,
											   bxdf::Result& result) const {
	Coating::importance_sample(wo, internal_ior, *this, sampler,
							   fresnel_result, result);
}

}}}
