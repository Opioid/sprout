#ifndef SU_CORE_SCENE_MATERIAL_NULL_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_NULL_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::null {

class Sample final : public material::Sample {

public:

	virtual const Layer& base_layer() const override final;

	virtual bxdf::Result evaluate(const float3& wi) const override final;

	virtual void sample(sampler::Sampler& sampler, bxdf::Sample& result) const override final;

	virtual BSSRDF bssrdf() const override final;

	virtual float ior() const override final;

	void set(const float3& absorption_coefficient, const float3& scattering_coefficient,
			 float anisotropy);

private:

	struct Layer : public material::Sample::Layer {
		BSSRDF bssrdf;
	};

	Layer layer_;

};

}

#endif
