#pragma once

#include "substitute_base_sample.hpp"
#include "scene/material/disney/disney.hpp"

namespace scene { namespace material { namespace substitute {

class Sample_subsurface : public Sample_base<disney::Isotropic_no_lambert> {

public:

	virtual float3 evaluate(const float3& wi, float& pdf) const override final;

	virtual void sample(sampler::Sampler& sampler, bxdf::Result& result) const override final;

	virtual float3 absorption_coeffecient() const override final;

	virtual const BSSRDF& bssrdf(Worker& worker) const override final;

	void set(const float3& absorption_coefficient, const float3& scattering_coefficient,
			 float ior, float ior_outside);

	virtual bool is_sss() const override final;

private:

	void refract(bool same_side, const Layer& layer, sampler::Sampler& sampler,
				 bxdf::Result& result) const;

	void reflect_internally(bool same_side, const Layer& layer, sampler::Sampler& sampler,
						   bxdf::Result& result) const;

	float3 absorption_coefficient_;
	float3 scattering_coefficient_;

	struct IOR {
		float ior_i_;
		float ior_o_;
		float eta_i_;
		float eta_t_;
	};

	IOR ior_;
};

}}}

