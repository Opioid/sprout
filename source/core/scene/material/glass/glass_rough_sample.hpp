#ifndef SU_CORE_SCENE_MATERIAL_GLASS_ROUGH_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_GLASS_ROUGH_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::glass {

class Sample_rough : public material::Sample {

public:

	virtual const Layer& base_layer() const override final;

	virtual bxdf::Result evaluate(const float3& wi) const override final;

	virtual void sample(sampler::Sampler& sampler, bxdf::Sample& result) const override final;

	virtual float3 absorption_coefficient() const override final;

	virtual float ior() const override final;

	virtual bool is_transmissive() const override final;

//	virtual bool is_translucent() const override final;

	virtual bool is_absorbing() const override final;

	struct Layer : public material::Sample::Layer {
		void set(const float3& refraction_color, const float3& absorption_color,
				 float attenuation_distance, float ior, float ior_outside, float alpha);

		float3 color_;
		float3 absorption_coefficient_;
		float ior_i_;
		float ior_o_;
		float eta_i_;
		float eta_t_;
		float f0_;
		float alpha_;
		float alpha2_;
	};

	Layer layer_;

	class BSDF {

	public:

		static float reflect(const Sample& sample, const Layer& layer,
							 sampler::Sampler& sampler, bxdf::Sample& result);

		static float refract(const Sample& sample, const Layer& layer,
							 sampler::Sampler& sampler, bxdf::Sample& result);
	};
};

}

#endif
