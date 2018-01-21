#ifndef SU_CORE_SCENE_MATERIAL_GLASS_DISPERSION_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_GLASS_DISPERSION_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::glass {

class Sample_dispersion final : public material::Sample {

public:

	virtual const Layer& base_layer() const override final;

	virtual bxdf::Result evaluate(const float3& wi) const override final;

	virtual float3 absorption_coefficient() const override final;

	virtual float ior() const override final;

	virtual void sample(sampler::Sampler& sampler, bxdf::Sample& result) const override final;

	virtual bool is_transmissive() const override final;

	void set(float abbe, float wavelength);

	struct Layer : public material::Sample::Layer {
		void set(const float3& refraction_color, const float3& absorption_color,
				 float attenuation_distance, float ior, float ior_outside);

		float3 color_;
		float3 absorption_coefficient_;
		mutable float ior_;
		float ior_outside_;
	};

	Layer layer_;

	class BSDF {

	public:

		static float reflect(const Sample_dispersion& sample, const Layer& layer,
							 sampler::Sampler& sampler, bxdf::Sample& result);

		static float refract(const Sample_dispersion& sample, const Layer& layer,
							 sampler::Sampler& sampler, bxdf::Sample& result);
	};

	float abbe_;
	float wavelength_;
};

}

#endif

