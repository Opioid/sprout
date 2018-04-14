#ifndef SU_SCENE_MATERIAL_LIGHT_SAMPLE_HPP
#define SU_SCENE_MATERIAL_LIGHT_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::light {

class Sample : public material::Sample {

public:

	virtual const Layer& base_layer() const override final;

	virtual bxdf::Result evaluate(const float3& wi) const override final;

	virtual float3 radiance() const override final;

	virtual void sample(sampler::Sampler& sampler, bxdf::Sample& result) const override final;

	virtual bool is_pure_emissive() const override final;

	struct Layer : public material::Sample::Layer {
		void set(const float3& radiance);

		float3 radiance_;
	};

	Layer layer_;
};

}

#endif
