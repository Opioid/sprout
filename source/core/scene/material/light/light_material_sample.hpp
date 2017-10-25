#pragma once

#include "scene/material/material_sample.hpp"

namespace scene::material::light {

class Sample : public material::Sample {

public:

	virtual const Layer& base_layer() const override final;

	virtual float3 evaluate(const float3& wi, float& pdf) const override final;

	virtual float3 radiance() const override final;

	virtual float ior() const override final;

	virtual void sample(sampler::Sampler& sampler, bxdf::Sample& result) const override final;

	virtual bool is_pure_emissive() const override final;

	struct Layer : public material::Sample::Layer {
		void set(const float3& radiance);

		float3 radiance_;
	};

	Layer layer_;
};

}
