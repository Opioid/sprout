#pragma once

#include "scene/material/material_sample.hpp"

namespace scene { namespace material { namespace coating {

struct Coating_base {
	void set_weight(float weight);

	float weight;
};

struct Clearcoat : public Coating_base {
	void set(float3_p color, float f0, float a2);

	template<typename Layer>
	float3 evaluate(float3_p wi, float3_p wo, float internal_ior,
					const Layer& layer, float3& attenuation, float& pdf) const;

	template<typename Layer>
	void importance_sample(float3_p wo, float internal_ior,
						   const Layer& layer, sampler::Sampler& sampler,
						   float3& attenuation, bxdf::Result& result) const;

	float f0;
	float a2;

	float3 color;
};

struct Thinfilm : public Coating_base {
	void set(float ior, float a2, float thickness);

	template<typename Layer>
	float3 evaluate(float3_p wi, float3_p wo, float internal_ior,
					const Layer& layer, float3& attenuation, float& pdf) const;

	template<typename Layer>
	void importance_sample(float3_p wo, float internal_ior,
						   const Layer& layer, sampler::Sampler& sampler,
						   float3& attenuation, bxdf::Result& result) const;

	float ior;
	float a2;
	float thickness;
};

template<typename Coating>
struct Coating_layer : Sample::Layer, Coating {
	float3 evaluate(float3_p wi, float3_p wo, float internal_ior,
					float3& attenuation, float& pdf) const;

	void importance_sample(float3_p wo, float internal_ior,
						   sampler::Sampler& sampler,
						   float3& attenuation, bxdf::Result& result) const;
};

using Clearcoat_layer = Coating_layer<Clearcoat>;
using Thinfilm_layer  = Coating_layer<Thinfilm>;

}}}
