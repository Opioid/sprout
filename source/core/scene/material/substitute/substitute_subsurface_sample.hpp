#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_SUBSURFACE_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_SUBSURFACE_SAMPLE_HPP

#include "substitute_base_sample.hpp"
#include "scene/material/disney/disney.hpp"

namespace scene::material::substitute {

class Sample_subsurface final : public Sample_base<disney::Isotropic_no_lambert> {

public:

	virtual bxdf::Result evaluate(f_float3 wi) const override final;

	virtual void sample(sampler::Sampler& sampler, bxdf::Sample& result) const override final;

	struct IOR {
		float ior_i_;
		float ior_o_;
		float eta_i_;
		float eta_t_;
	};

	void set(float anisotropy, IOR const& ior);

private:

	void refract(bool same_side, Layer const& layer, sampler::Sampler& sampler,
				 bxdf::Sample& result) const;

	void reflect_internally(Layer const& layer, sampler::Sampler& sampler,
							bxdf::Sample& result) const;

	float anisotropy_;

	IOR ior_;
};

}

#endif
