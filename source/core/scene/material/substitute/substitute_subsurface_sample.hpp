#pragma once

#include "substitute_base_sample.hpp"
#include "scene/material/disney/disney.hpp"

namespace scene { namespace material { namespace substitute {

class Sample_subsurface : public Sample_base<disney::Isotropic_no_lambert> {

public:

	virtual float3 evaluate(const float3& wi, float& pdf) const final override;

	virtual void sample(sampler::Sampler& sampler, bxdf::Result& result) const final override;

	void set(float ior, float ior_outside);

private:

	void refract(bool same_side, sampler::Sampler& sampler, bxdf::Result& result) const;

	struct IOR {
		float ior_i_;
		float ior_o_;
		float eta_i_;
		float eta_t_;
	};

	IOR ior_;
};

}}}

