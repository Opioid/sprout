#pragma once

#include "substitute_base_sample.hpp"
#include "scene/material/disney/disney.hpp"

namespace scene { namespace material { namespace substitute {

class Sample_translucent : public Sample_base<disney::Isotropic> {

public:

	virtual float3 evaluate(const float3& wi, float& pdf) const final override;

	virtual void sample(sampler::Sampler& sampler, bxdf::Result& result) const final override;

	virtual bool is_translucent() const final override;

	void set(const float3& diffuse_color, float thickness, float attenuation_distance);

private:

	float3 attenuation_;
	float thickness_;
};

}}}
