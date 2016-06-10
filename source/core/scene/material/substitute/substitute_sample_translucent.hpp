#pragma once

#include "substitute_sample_base.hpp"
#include "scene/material/lambert/lambert.hpp"

namespace scene { namespace material { namespace substitute {

class Sample_translucent : public Sample_base {

public:

	virtual math::float3 evaluate(math::pfloat3 wi, float& pdf) const final override;

	virtual void sample_evaluate(sampler::Sampler& sampler,
								 bxdf::Result& result) const final override;

	virtual bool is_translucent() const final override;

	void set(math::pfloat3 color, math::pfloat3 radiance,
			 float constant_f0, float a2, float metallic,
			 float thickness, float attenuation_distance);

private:

	math::float3 attenuation_;
	float thickness_;

	friend lambert::Isotropic;
};

}}}
