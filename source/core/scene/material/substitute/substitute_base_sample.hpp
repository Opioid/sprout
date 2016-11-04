#pragma once

#include "scene/material/material_sample.hpp"

namespace scene { namespace material { namespace substitute {

class Sample_base : public material::Sample {

public:

	virtual const Layer& base_layer() const final override;

	virtual float3 radiance() const final override;

	virtual float3 attenuation() const final override;

	virtual float ior() const final override;

	virtual bool is_pure_emissive() const final override;

	virtual bool is_transmissive() const final override;

	virtual bool is_translucent() const override;

protected:

	template<typename Coating>
	float3 base_evaluate_and_coating(float3_p wi, const Coating& coating,
									 float& pdf) const;

	template<typename Coating>
	void base_sample_and_coating(const Coating& coating,
								 sampler::Sampler& sampler,
								 bxdf::Result& result) const;

	template<typename Coating>
	void diffuse_sample_and_coating(const Coating& coating,
									sampler::Sampler& sampler,
									bxdf::Result& result) const;

	template<typename Coating>
	void specular_sample_and_coating(const Coating& coating,
									 sampler::Sampler& sampler,
									 bxdf::Result& result) const;

	template<typename Coating>
	void pure_specular_sample_and_coating(const Coating& coating,
										  sampler::Sampler& sampler,
										  bxdf::Result& result) const;

public:

	struct Layer : material::Sample::Layer {
		void set(float3_p color, float3_p radiance, float ior,
				 float constant_f0, float roughness, float metallic);

		float3 base_evaluate(float3_p wi, float3_p wo, float& pdf) const;

		void diffuse_sample(float3_p wo, sampler::Sampler& sampler,
							bxdf::Result& result) const;

		void specular_sample(float3_p wo, sampler::Sampler& sampler,
							 bxdf::Result& result) const;

		void pure_specular_sample(float3_p wo, sampler::Sampler& sampler,
								  bxdf::Result& result) const;

		float3 diffuse_color;
		float3 f0;
		float3 emission;

		float ior;
		float roughness;
		float a2;
		float metallic;
	};

	Layer layer_;
};

}}}
