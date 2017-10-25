#pragma once

#include "scene/material/material_sample.hpp"

namespace scene::material::display {

class Sample : public material::Sample {

public:

	virtual const Layer& base_layer() const override final;

	virtual float3 evaluate(const float3& wi, float& pdf) const override final;

	virtual float3 radiance() const override final;

	virtual float ior() const override final;

	virtual void sample(sampler::Sampler& sampler, bxdf::Sample& result) const override final;

	struct Layer : material::Sample::Layer {
		void set(const float3& radiance, float f0, float roughness);

		float3 emission_;
		float3 f0_;
		float alpha_;
		float alpha2_;
	};

	Layer layer_;
};

}
