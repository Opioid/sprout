#pragma once

#include "scene/material/material_sample.hpp"

namespace scene::material::substitute {

template<typename Diffuse>
class Sample_base : public material::Sample {

public:

	virtual const Layer& base_layer() const override final;

	virtual float3 radiance() const override final;

	virtual float ior() const override final;

protected:

	template<typename Coating>
	float3 base_and_coating_evaluate(const float3& wi, const Coating& coating, float& pdf) const;

	template<typename Coating>
	void base_and_coating_sample(const Coating& coating, sampler::Sampler& sampler,
								 bxdf::Result& result) const;

	template<typename Coating>
	void diffuse_sample_and_coating(const Coating& coating, sampler::Sampler& sampler,
									bxdf::Result& result) const;

	template<typename Coating>
	void specular_sample_and_coating(const Coating& coating, sampler::Sampler& sampler,
									 bxdf::Result& result) const;

	template<typename Coating>
	void pure_specular_sample_and_coating(const Coating& coating, sampler::Sampler& sampler,
										  bxdf::Result& result) const;

public:

	struct Layer : material::Sample::Layer {
		void set(const float3& color, const float3& radiance, float ior,
				 float constant_f0, float roughness, float metallic);

		float3 base_evaluate(const float3& wi, const float3& wo, const float3& h,
							 float wo_dot_h, float& pdf) const;

		void diffuse_sample(const float3& wo, sampler::Sampler& sampler,
							bxdf::Result& result) const;

		void specular_sample(const float3& wo, sampler::Sampler& sampler,
							 bxdf::Result& result) const;

		void pure_specular_sample(const float3& wo, sampler::Sampler& sampler,
								  bxdf::Result& result) const;

		float base_diffuse_fresnel_hack(float n_dot_wi, float n_dot_wo) const;

		float3 diffuse_color_;
		float3 f0_;
		float3 emission_;

		float ior_;
		float roughness_;
		float alpha_;
		float alpha2_;
		float metallic_;
	};

	Layer layer_;
};

}
