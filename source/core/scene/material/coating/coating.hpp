#pragma once

#include "scene/material/material_sample.hpp"

namespace scene { namespace material { namespace coating {

struct Coating_base {
	void set_color_and_weight(const float3& color, float weight);

	float3 color_;
	float  weight_;
};

struct Clearcoat : public Coating_base {
	void set(float f0, float alpha, float alpha2);

	template<typename Layer>
	float3 evaluate(const float3& wi, const float3& wo, const float3& h,
					float wo_dot_h, float internal_ior,
					const Layer& layer, float3& attenuation, float& pdf) const;

	template<typename Layer>
	void sample(const float3& wo, float internal_ior,
				const Layer& layer, sampler::Sampler& sampler,
				float3& attenuation, bxdf::Result& result) const;

	float f0_;
	float alpha_;
	float alpha2_;
};

struct Thinfilm : public Coating_base {
	void set(float ior, float alpha, float alpha2, float thickness);

	template<typename Layer>
	float3 evaluate(const float3& wi, const float3& wo, const float3& h,
					float wo_dot_h, float internal_ior,
					const Layer& layer, float3& attenuation, float& pdf) const;

	template<typename Layer>
	void sample(const float3& wo, float internal_ior,
				const Layer& layer, sampler::Sampler& sampler,
				float3& attenuation, bxdf::Result& result) const;

	float ior_;
	float alpha_;
	float alpha2_;
	float thickness_;
};

template<typename Coating>
struct Coating_layer : Sample::Layer, Coating {
	float3 evaluate(const float3& wi, const float3& wo, const float3& h, float wo_dot_h,
					float internal_ior, float3& attenuation, float& pdf) const;

	void sample(const float3& wo, float internal_ior, sampler::Sampler& sampler,
				float3& attenuation, bxdf::Result& result) const;
};

using Clearcoat_layer = Coating_layer<Clearcoat>;
using Thinfilm_layer  = Coating_layer<Thinfilm>;

}}}
