#pragma once

#include "coating.hpp"
#include "scene/material/bxdf.hpp"

namespace scene { namespace material { namespace coating {

inline void Clearcoat::set(float f0, float a2, float weight) {
	this->f0 = f0;
	this->a2 = a2;
	this->weight = weight;
}

template<typename Layer>
void Clearcoat::pure_specular_importance_sample(float3_p wo, float n_dot_wo, float /*internal_ior*/,
												const Layer& layer, sampler::Sampler& sampler,
												bxdf::Result& result) const {
	fresnel::Schlick schlick(f0);
	float n_dot_wi = ggx::Isotropic::importance_sample(wo, n_dot_wo, layer,
													   schlick, sampler, result);
	result.reflection *= n_dot_wi;
}

inline void Thinfilm::set(float ior, float a2, float thickness, float weight) {
	this->ior = ior;
	this->a2  = a2;
	this->thickness = thickness;
	this->weight = weight;
}

template<typename Layer>
void Thinfilm::pure_specular_importance_sample(float3_p wo, float n_dot_wo, float internal_ior,
											   const Layer& layer, sampler::Sampler& sampler,
											   bxdf::Result& result) const {
	fresnel::Thinfilm_weighted thinfilm(1.f, ior, internal_ior,thickness, weight);
	float n_dot_wi = ggx::Isotropic::importance_sample(wo, n_dot_wo, layer,
													   thinfilm, sampler, result);
	result.reflection *= n_dot_wi;
}

template<typename Coating>
void Coating_layer<Coating>::pure_specular_importance_sample(float3_p wo, float n_dot_wo,
															 float internal_ior,
															 sampler::Sampler& sampler,
															 bxdf::Result& result) const {
	pure_specular_importance_sample(wo, n_dot_wo, internal_ior, *this, sampler, result);
}

}}}
