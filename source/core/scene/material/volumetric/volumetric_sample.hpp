#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::volumetric {

class Sample final : public material::Sample {

public:

	virtual const Layer& base_layer() const override final;

	virtual bxdf::Result evaluate(const float3& wi) const override final;

	virtual void sample(sampler::Sampler& sampler, bxdf::Sample& result) const override final;

	virtual BSSRDF bssrdf() const override final;

	virtual bool is_translucent() const override final;

	void set(const float3& absorption_coefficient, const float3& scattering_coefficient,
			 float anisotropy);

public:

	struct Layer : public material::Sample::Layer {
		float phase(const float3& wo, const float3& wi) const;

		float sample(const float3& wo, float2 r2, float3& wi) const;

		BSSRDF bssrdf;
	};

	Layer layer_;

private:

	static float phase_hg(float cos_theta, float g);

	static float phase_schlick(float cos_theta, float k);
};

}

#endif
