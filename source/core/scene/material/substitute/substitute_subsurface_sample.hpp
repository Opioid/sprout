#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_SUBSURFACE_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_SUBSURFACE_SAMPLE_HPP

#include "substitute_base_sample.hpp"
#include "scene/material/disney/disney.hpp"

namespace scene::material::substitute {

class Sample_subsurface final : public Sample_base<disney::Isotropic_no_lambert> {

public:

	virtual bxdf::Result evaluate(const float3& wi) const override final;

	virtual void sample(sampler::Sampler& sampler, bxdf::Sample& result) const override final;

	virtual float3 absorption_coefficient() const override final;

	struct IOR {
		float ior_i_;
		float ior_o_;
		float eta_i_;
		float eta_t_;
	};

	void set(float anisotropy, const IOR& ior);

private:

	void refract(bool same_side, const Layer& layer, sampler::Sampler& sampler,
				 bxdf::Sample& result) const;

	void reflect_internally(const Layer& layer, sampler::Sampler& sampler,
							bxdf::Sample& result) const;

	float anisotropy_;

	IOR ior_;
};

}

#endif
