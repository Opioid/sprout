#pragma once

#include "substitute_base_sample.hpp"
#include "scene/material/coating/coating.hpp"
#include "scene/material/disney/disney.hpp"

namespace scene::material::substitute {

template<typename Coating_layer>
class Sample_coating : public Sample_base<disney::Isotropic> {

public:

	virtual float3 evaluate(const float3& wi, float& pdf) const override final;

	virtual void sample(sampler::Sampler& sampler, bxdf::Result& result) const override final;

	Coating_layer coating_;
};

using Sample_clearcoat = Sample_coating<coating::Clearcoat_layer>;
using Sample_thinfilm  = Sample_coating<coating::Thinfilm_layer>;

}
