#pragma once

#include "scene/material/material_sample.hpp"

namespace scene { namespace material { namespace display {

class Sample : public material::Sample {

public:

	virtual const Layer& base_layer() const final override;

	virtual float3 evaluate(const float3& wi, float& pdf) const final override;

	virtual float3 radiance() const final override;

	virtual float ior() const final override;

	virtual void sample(sampler::Sampler& sampler, bxdf::Result& result) const final override;

	struct Layer : material::Sample::Layer {
		void set(const float3& radiance, float f0, float roughness);

		float3 emission_;
		float3 f0_;
		float a2_;
	};

	Layer layer_;
};

}}}
