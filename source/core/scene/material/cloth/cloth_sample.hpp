#pragma once

#include "scene/material/material_sample.hpp"
#include "scene/material/lambert/lambert.hpp"

namespace scene { namespace material { namespace cloth {

class Sample : public material::Sample {

public:

	virtual const Layer& base_layer() const final override;

	virtual float3 evaluate(const float3& wi, float& pdf) const final override;

	virtual float3 radiance() const final override;

	virtual float3 attenuation() const final override;

	virtual float ior() const final override;

	virtual void sample(sampler::Sampler& sampler,
								 bxdf::Result& result) const final override;

	virtual bool is_pure_emissive() const final override;

	virtual bool is_transmissive() const final override;

	virtual bool is_translucent() const final override;

	struct Layer : public material::Sample::Layer {
		void set(const float3& color);

		float3 diffuse_color;
	};

	Layer layer_;
};

}}}
