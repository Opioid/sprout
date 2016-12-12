#pragma once

#include "scene/material/material_sample.hpp"
#include "scene/material/coating/coating.hpp"

namespace scene { namespace material { namespace metallic_paint {

class Sample : public material::Sample {

public:

	virtual const Layer& base_layer() const final override;

	virtual float3 evaluate(float3_p wi, float& pdf) const final override;

	virtual void sample(sampler::Sampler& sampler,
						bxdf::Result& result) const final override;

	virtual float3 radiance() const final override;

	virtual float3 attenuation() const final override;

	virtual float ior() const final override;

	virtual bool is_pure_emissive() const final override;

	virtual bool is_transmissive() const final override;

	virtual bool is_translucent() const final override;

	struct Base_layer : material::Sample::Layer {
		void set(float3_p color_a, float3_p color_b, float a2);

		float3 evaluate(float3_p wi, float3_p wo, float3_p h,
						float wo_dot_h, float& pdf) const;

		void sample(float3_p wo, sampler::Sampler& sampler, bxdf::Result& result) const;

		float3 color_a_;
		float3 color_b_;

		float a2_;
	};

	struct Flakes_layer : material::Sample::Layer {
		void set(float3_p ior, float3_p absorption, float a2, float weight);

		float3 evaluate(float3_p wi, float3_p wo, float3_p h,
						float wo_dot_h, float3& fresnel_result, float& pdf) const;

		void sample(float3_p wo, sampler::Sampler& sampler,
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

