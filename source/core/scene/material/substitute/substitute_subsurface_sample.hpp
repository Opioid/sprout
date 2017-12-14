#pragma once

#include "substitute_base_sample.hpp"
#include "scene/material/disney/disney.hpp"

namespace scene::material::substitute {

class Sample_subsurface :
		public Sample_base<disney::Isotropic_scaled_lambert,
						   disney::Isotropic_scaled_lambert::Data> {

public:

	virtual bxdf::Result evaluate(const float3& wi) const override final;

	virtual void sample(sampler::Sampler& sampler, bxdf::Sample& result) const override final;

	virtual float3 absorption_coeffecient() const override final;

	virtual const BSSRDF& bssrdf(const Worker& worker) const override final;

	struct IOR {
		float ior_i_;
		float ior_o_;
		float eta_i_;
		float eta_t_;
		float sqrt_eta_i;
		float sqrt_eta_t;
	};

	void set(float lambert_scale,
			 const float3& absorption_coefficient, const float3& scattering_coefficient,
			 const IOR& ior);

	virtual bool is_sss() const override final;

private:

	void refract(bool same_side, const Layer& layer, sampler::Sampler& sampler,
				 bxdf::Sample& result) const;

	void reflect_internally(bool same_side, const Layer& layer, sampler::Sampler& sampler,
						   bxdf::Sample& result) const;

	float3 absorption_coefficient_;
	float3 scattering_coefficient_;

	IOR ior_;
};

}

