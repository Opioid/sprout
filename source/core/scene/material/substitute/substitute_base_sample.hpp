#pragma once

#include "scene/material/material_sample.hpp"
#include "scene/material/ggx/ggx.hpp"
#include "scene/material/oren_nayar/oren_nayar.hpp"

namespace scene { namespace material { namespace substitute {

class Sample_base : public material::Sample {

public:

	virtual math::float3 radiance() const final override;

	virtual math::float3 attenuation() const final override;

	virtual float ior() const final override;

	virtual bool is_pure_emissive() const final override;

	virtual bool is_transmissive() const final override;

	virtual bool is_translucent() const override;

	void set(math::pfloat3 color, math::pfloat3 radiance,
			 float ior, float constant_f0, float a2, float metallic);

protected:

	math::float3 base_evaluate(math::pfloat3 wi, float& pdf) const;

	void diffuse_importance_sample(sampler::Sampler& sampler, bxdf::Result& result) const;
	void specular_importance_sample(sampler::Sampler& sampler, bxdf::Result& result) const;
	void pure_specular_importance_sample(sampler::Sampler& sampler, bxdf::Result& result) const;

	template<typename Coating>
	math::float3 base_evaluate_and_coating(math::pfloat3 wi, const Coating& coating,
										   float coating_a2, float& pdf) const;

	template<typename Coating>
	void base_sample_evaluate_and_coating(const Coating& coating,
										  float coating_a2,
										  sampler::Sampler& sampler,
										  bxdf::Result& result) const;

	template<typename Coating>
	void diffuse_importance_sample_and_coating(const Coating& coating,
											   float coating_a2,
											   sampler::Sampler& sampler,
											   bxdf::Result& result) const;

	template<typename Coating>
	void specular_importance_sample_and_coating(const Coating& coating,
												float coating_a2,
												sampler::Sampler& sampler,
												bxdf::Result& result) const;

	template<typename Coating>
	void pure_specular_importance_sample_and_coating(const Coating& coating,
													 float coating_a2,
													 sampler::Sampler& sampler,
													 bxdf::Result& result) const;

	math::float3 diffuse_color_;
	math::float3 f0_;
	math::float3 emission_;

	float ior_;
	float a2_;
	float metallic_;

	friend oren_nayar::Isotropic;
	friend ggx::Isotropic;
};

}}}
