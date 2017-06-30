#pragma once

#include "scene/material/material_sample.hpp"

namespace scene { namespace material { namespace glass {

class Sample : public material::Sample {

public:

	virtual const Layer& base_layer() const final override;

	virtual float3 evaluate(const float3& wi, float& pdf) const final override;

	virtual float3 radiance() const final override;

	virtual float3 absorption_coffecient() const final override;

	virtual float ior() const final override;

	virtual void sample(sampler::Sampler& sampler, bxdf::Result& result) const final override;

	virtual bool is_pure_emissive() const final override;

	virtual bool is_transmissive() const final override;

	virtual bool is_translucent() const final override;

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
							 sampler::Sampler& sampler, bxdf::Result& result);

		static float refract(const Sample& sample, const Layer& layer,
							 sampler::Sampler& sampler, bxdf::Result& result);
	};
};

}}}
