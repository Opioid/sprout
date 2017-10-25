#pragma once

#include "scene/material/material_sample.hpp"

namespace scene::material::glass {

class Sample : public material::Sample {

public:

	virtual const Layer& base_layer() const override final;

	virtual float3 evaluate(const float3& wi, float& pdf) const override final;

	virtual float3 absorption_coeffecient() const override final;

	virtual float ior() const override final;

	virtual void sample(sampler::Sampler& sampler, bxdf::Sample& result) const override final;

	virtual bool is_transmissive() const override final;

	struct Layer : public material::Sample::Layer {
		void set(const float3& refraction_color, const float3& absorption_color,
				 float attenuation_distance, float ior, float ior_outside);

		float3 color_;
		float3 absorption_coffecient_;
		float ior_;
		float ior_outside_;
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
