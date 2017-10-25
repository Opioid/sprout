#pragma once

#include "substitute_base_sample.hpp"
#include "scene/material/disney/disney.hpp"

namespace scene::material::substitute {

class Sample_translucent : public Sample_base<disney::Isotropic> {

public:

	virtual float3 evaluate(const float3& wi, float& pdf) const override final;

	virtual void sample(sampler::Sampler& sampler, bxdf::Sample& result) const override final;

	virtual bool is_translucent() const override final;

	void set(const float3& diffuse_color, float thickness, float attenuation_distance);

private:

	float3 attenuation_;
	float thickness_;
};

}
