#pragma once

#include "substitute_base_sample.hpp"
#include "scene/material/coating/coating.hpp"

namespace scene { namespace material { namespace substitute {

template<typename Coating_layer>
class Sample_coating : public Sample_base {

public:

	virtual float3 evaluate(float3_p wi, float& pdf) const final override;

	virtual void sample(sampler::Sampler& sampler,
								 bxdf::Result& result) const final override;

	Coating_layer coating_;
};

using Sample_clearcoat = Sample_coating<coating::Clearcoat_layer>;
using Sample_thinfilm  = Sample_coating<coating::Thinfilm_layer>;

}}}
