#pragma once

#include "substitute_base_sample.hpp"
#include "scene/material/disney/disney.hpp"

namespace scene::material::substitute {

class Sample : public Sample_base<disney::Isotropic> {

public:

	virtual float3 evaluate(const float3& wi, float& pdf) const override final;

	virtual void sample(sampler::Sampler& sampler, bxdf::Sample& result) const override final;
};

}
