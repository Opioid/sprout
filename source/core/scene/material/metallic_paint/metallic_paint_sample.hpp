#pragma once

#include "scene/material/material_sample.hpp"

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

	struct Layer : material::Sample::Layer {
		void set(float3_p color_a, float3_p color_b);
		float3 color_a;
		float3 color_b;
	};

	Layer layer_;
};

}}}

