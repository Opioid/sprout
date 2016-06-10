#pragma once

#include "substitute_base_sample.hpp"

namespace scene { namespace material { namespace substitute {

class Sample : public Sample_base {

public:

	virtual math::float3 evaluate(math::pfloat3 wi, float& pdf) const final override;

	virtual void sample_evaluate(sampler::Sampler& sampler,
								 bxdf::Result& result) const final override;

	void set(math::pfloat3 color, math::pfloat3 radiance,
			 float constant_f0, float a2, float metallic);
};

}}}
