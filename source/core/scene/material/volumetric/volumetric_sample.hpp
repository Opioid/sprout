#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_SAMPLE_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_SAMPLE_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::volumetric {

class Sample final : public material::Sample {

public:

	virtual const Layer& base_layer() const override final;

	virtual bxdf::Result evaluate(const float3& wi) const override final;

	virtual void sample(sampler::Sampler& sampler, bxdf::Sample& result) const override final;

	virtual float ior() const override final;

	virtual bool is_translucent() const override final;

public:

	struct Layer : public material::Sample::Layer {
		float phase(const float3& w, const float3& wp) const;

		void set(float anisotropy);

		float anisotropy_;
	};

	Layer layer_;

private:

	static float phase_schlick(const float3& w, const float3& wp, float k);
};

}

#endif