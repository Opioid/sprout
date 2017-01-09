#pragma once

#include "substitute_base_sample.hpp"
#include "scene/material/disney/disney.hpp"

namespace scene { namespace material { namespace substitute {

class Sample_subsurface : public Sample_base<disney::Isotropic_no_lambert> {

public:

	virtual float3 evaluate(float3_p wi, float& pdf) const final override;

	virtual void sample(sampler::Sampler& sampler,
						bxdf::Result& result) const final override;
};

}}}

