#pragma once

#include "substitute_base_sample.hpp"
#include "scene/material/lambert/lambert.hpp"

namespace scene { namespace material { namespace substitute {

class Sample_translucent : public Sample_base {

public:

	virtual float3 evaluate(float3_p wi, float& pdf) const final override;

	virtual void sample_evaluate(sampler::Sampler& sampler,
								 bxdf::Result& result) const final override;

	virtual bool is_translucent() const final override;

	void set(float3_p color, float metallic,
			 float thickness, float attenuation_distance);

private:

	float3 attenuation_;
	float thickness_;
};

}}}
