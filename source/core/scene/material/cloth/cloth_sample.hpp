#pragma once

#include "scene/material/material_sample.hpp"

namespace scene { namespace material { namespace cloth {

class Sample : public material::Sample {

public:

	virtual const Layer& base_layer() const override final;

	virtual float3 evaluate(const float3& wi, float& pdf) const override final;

	virtual float ior() const override final;

	virtual void sample(sampler::Sampler& sampler, bxdf::Result& result) const override final;

	struct Layer : public material::Sample::Layer {
		void set(const float3& color);

		float3 diffuse_color;
	};

	Layer layer_;
};

}}}
