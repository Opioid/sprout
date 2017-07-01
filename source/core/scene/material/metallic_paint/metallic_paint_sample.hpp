#pragma once

#include "scene/material/material_sample.hpp"
#include "scene/material/coating/coating.hpp"

namespace scene { namespace material { namespace metallic_paint {

class Sample : public material::Sample {

public:

	virtual const Layer& base_layer() const final override;

	virtual float3 evaluate(const float3& wi, float& pdf) const final override;

	virtual void sample(sampler::Sampler& sampler, bxdf::Result& result) const final override;

	virtual float ior() const final override;

	struct Base_layer : material::Sample::Layer {
		void set(const float3& color_a, const float3& color_b, float a2);

		float3 evaluate(const float3& wi, const float3& wo, const float3& h,
						float wo_dot_h, float& pdf) const;

		void sample(const float3& wo, sampler::Sampler& sampler, bxdf::Result& result) const;

		float3 color_a_;
		float3 color_b_;

		float a2_;
	};

	struct Flakes_layer : material::Sample::Layer {
		void set(const float3& ior, const float3& absorption, float a2, float weight);

		float3 evaluate(const float3& wi, const float3& wo, const float3& h,
						float wo_dot_h, float3& fresnel_result, float& pdf) const;

		void sample(const float3& wo, sampler::Sampler& sampler,
					float3& fresnel_result, bxdf::Result& result) const;

		float3 ior_;
		float3 absorption_;

		float a2_;
		float weight_;
	};

	Base_layer base_;

	Flakes_layer flakes_;

	coating::Clearcoat_layer coating_;
};

}}}

