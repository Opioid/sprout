#pragma once

#include "scene/material/material_sample.hpp"

namespace scene { namespace material { namespace coating {

struct Clearcoat {
	void set(float f0, float a2, float weight);

	template<typename Layer>
	void pure_specular_importance_sample(float3_p wo, float n_dot_wo, float internal_ior,
										 const Layer& layer, sampler::Sampler& sampler,
										 bxdf::Result& result) const;

	float f0;
	float a2;
	float weight;
};

struct Thinfilm {
	void set(float ior, float a2, float thickness, float weight);

	template<typename Layer>
	void pure_specular_importance_sample(float3_p wo, float n_dot_wo, float internal_ior,
										 const Layer& layer, sampler::Sampler& sampler,
										 bxdf::Result& result) const;

	float ior;
	float a2;
	float thickness;
	float weight;
};

template<typename Coating>
struct Coating_layer : Sample::Layer, Coating {
	void pure_specular_importance_sample(float3_p wo, float n_dot_wo, float internal_ior,
										 sampler::Sampler& sampler, bxdf::Result& result) const;
};

using Clearcoat_layer = Coating_layer<Clearcoat>;
using Thinfilm_layer  = Coating_layer<Thinfilm>;

}}}
