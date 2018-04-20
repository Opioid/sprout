#ifndef SU_CORE_SCENE_MATERIAL_GLASS_ROUGH_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_GLASS_ROUGH_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::glass {

class Sample_rough final : public material::Sample {

public:

	virtual const Layer& base_layer() const override final;

	virtual bxdf::Result evaluate(const float3& wi) const override final;

	virtual void sample(sampler::Sampler& sampler, bxdf::Sample& result) const override final;

	virtual bool is_transmissive() const override final;

	struct IOR {
		float ior_i_;
		float ior_o_;
		float eta_i_;
		float eta_t_;
	};

	void set(const float3& refraction_color, const float3& absorption_color,
			 float attenuation_distance, float ior, float ior_outside, float alpha);

	struct Layer : public material::Sample::Layer {
		float3 color_;
		float3 absorption_coefficient_;
		float f0_;
		float alpha_;
		float alpha2_;
	};

	Layer layer_;

	IOR ior_;

	void reflect(const Layer& layer, sampler::Sampler& sampler,
				 bxdf::Sample& result) const;

	void reflect_internally(const Layer& layer, sampler::Sampler& sampler,
							bxdf::Sample& result) const;

	void refract(bool same_side, const Layer& layer, sampler::Sampler& sampler,
				 bxdf::Sample& result) const;
};

}

#endif
