#pragma once

#include "scene/material/material_sample.hpp"
#include "scene/material/coating/coating.hpp"

namespace scene { namespace material { namespace metallic_paint {

class Sample : public material::Sample {

public:

	virtual float3_p shading_normal() const final override;

	virtual float3 tangent_to_world(float3_p v) const final override;

	virtual float3 evaluate(float3_p wi, float& pdf) const final override;

	virtual float3 radiance() const final override;

	virtual float3 attenuation() const final override;

	virtual float ior() const final override;

	virtual void sample_evaluate(sampler::Sampler& sampler,
								 bxdf::Result& result) const final override;

	virtual bool is_pure_emissive() const final override;

	virtual bool is_transmissive() const final override;

	virtual bool is_translucent() const final override;

	struct Base_layer : material::Sample::Layer {
		void set(float3_p color_a, float3_p color_b);

		float3 evaluate(float3_p wi, float3_p wo, float& pdf) const;

		void importance_sample(float3_p wo, sampler::Sampler& sampler,
							   bxdf::Result& result) const;

		float3 color_a;
		float3 color_b;

		float a2;
	};

	struct Flakes_layer : material::Sample::Layer {
		float3 evaluate(float3_p wi, float3_p wo, float3_p base_n, float& weight, float& pdf) const;

		void importance_sample(float3_p wo, float3_p base_n, sampler::Sampler& sampler,
							   float& weight, bxdf::Result& result) const;

		float3 ior;
		float3 absorption;

		float a2;
	};

	Base_layer base_;

	Flakes_layer flakes_;

	coating::Clearcoat_layer coating_;
};

}}}

