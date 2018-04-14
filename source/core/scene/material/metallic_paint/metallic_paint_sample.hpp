#pragma once

#include "scene/material/material_sample.hpp"
#include "scene/material/coating/coating.hpp"

namespace scene::material::metallic_paint {

class Sample : public material::Sample {

public:

	virtual const Layer& base_layer() const override final;

	virtual bxdf::Result evaluate(const float3& wi) const override final;

	virtual void sample(sampler::Sampler& sampler, bxdf::Sample& result) const override final;

	struct Base_layer : material::Sample::Layer {
		void set(const float3& color_a, const float3& color_b, float alpha, float alpha2);

		bxdf::Result evaluate(const float3& wi, const float3& wo, const float3& h,
							  float wo_dot_h) const;

		void sample(const float3& wo, sampler::Sampler& sampler, bxdf::Sample& result) const;

		float3 color_a_;
		float3 color_b_;

		float alpha_;
		float alpha2_;
	};

	struct Flakes_layer : material::Sample::Layer {
		void set(const float3& ior, const float3& absorption,
				 float alpha, float alpha2, float weight);

		bxdf::Result evaluate(const float3& wi, const float3& wo, const float3& h,
							  float wo_dot_h, float3& fresnel_result) const;

		void sample(const float3& wo, sampler::Sampler& sampler,
					float3& fresnel_result, bxdf::Sample& result) const;

		float3 ior_;
		float3 absorption_;

		float alpha_;
		float alpha2_;
		float weight_;
	};

	Base_layer base_;

	Flakes_layer flakes_;

	coating::Clearcoat_layer coating_;
};

}
