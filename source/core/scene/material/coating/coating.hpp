#pragma once

#include "scene/material/material_sample.hpp"

namespace scene { namespace material { namespace coating {

struct Clearcoat {
	void set(float f0, float a2, float weight);

	template<typename Layer>
	float3 evaluate(float3_p wi, float3_p wo, float internal_ior, const Layer& layer,
					float3& fresnel_result, float& pdf) const;

	template<typename Layer>
	void importance_sample(float3_p wo, float internal_ior,
						   const Layer& layer, sampler::Sampler& sampler,
						   float3& fresnel_result, bxdf::Result& result) const;

	float f0;
	float a2;
	float weight;
};

struct Thinfilm {
	void set(float ior, float a2, float thickness, float weight);

	template<typename Layer>
	float3 evaluate(float3_p wi, float3_p wo, float internal_ior, const Layer& layer,
					float3& fresnel_result, float& pdf) const;

	template<typename Layer>
	void importance_sample(float3_p wo, float internal_ior,
						   const Layer& layer, sampler::Sampler& sampler,
						   float3& fresnel_result, bxdf::Result& result) const;

	float ior;
	float a2;
	float thickness;
	float weight;
};

template<typename Coating>
struct Coating_layer : Sample::Layer, Coating {
	float3 evaluate(float3_p wi, float3_p wo, float internal_ior,
					float3& fresnel_result, float& pdf) const;

	void importance_sample(float3_p wo, float internal_ior,
						   sampler::Sampler& sampler,
						   float3& fresnel_result, bxdf::Result& result) const;
};

using Clearcoat_layer = Coating_layer<Clearcoat>;
using Thinfilm_layer  = Coating_layer<Thinfilm>;

}}}
