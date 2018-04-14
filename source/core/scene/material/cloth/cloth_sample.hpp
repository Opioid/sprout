#pragma once

#include "scene/material/material_sample.hpp"

namespace scene::material::cloth {

class Sample : public material::Sample {

public:

	virtual const Layer& base_layer() const override final;

	virtual bxdf::Result evaluate(const float3& wi) const override final;

	virtual void sample(sampler::Sampler& sampler, bxdf::Sample& result) const override final;

	struct Layer : public material::Sample::Layer {
		void set(const float3& color);

		float3 diffuse_color;
	};

	Layer layer_;
};

}
