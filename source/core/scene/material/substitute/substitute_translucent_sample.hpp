#pragma once

#include "substitute_base_sample.hpp"
#include "scene/material/disney/disney.hpp"

namespace scene::material::substitute {

class Sample_translucent : public Sample_base<disney::Isotropic> {

public:

	virtual bxdf::Result evaluate(f_float3 wi) const override final;

	virtual void sample(sampler::Sampler& sampler, bxdf::Sample& result) const override final;

	virtual bool is_translucent() const override final;

	void set(float3 const& diffuse_color, float thickness, float attenuation_distance);

private:

	float3 attenuation_;
	float thickness_;
};

}
