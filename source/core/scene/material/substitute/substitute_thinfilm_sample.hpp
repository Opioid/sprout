#pragma once

#include "substitute_base_sample.hpp"
#include "scene/material/coating/coating.hpp"

namespace scene { namespace material { namespace substitute {

class Sample_thinfilm : public Sample_base {

public:

	virtual float3 evaluate(float3_p wi, float& pdf) const final override;

	virtual void sample_evaluate(sampler::Sampler& sampler,
								 bxdf::Result& result) const final override;

	void set_thinfilm(const coating::Thinfilm& coating);

private:

	coating::Thinfilm coating_;
};

}}}
