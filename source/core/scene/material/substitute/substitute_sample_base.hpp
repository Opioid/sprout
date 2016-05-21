#pragma once

#include "scene/material/material_sample.hpp"
#include "scene/material/ggx/ggx.hpp"
#include "scene/material/oren_nayar/oren_nayar.hpp"

namespace scene { namespace material { namespace substitute {

class Sample_base : public material::Sample {

public:

	virtual math::float3 emission() const final override;

	virtual math::float3 attenuation() const final override;

	virtual float ior() const final override;

	virtual bool is_pure_emissive() const final override;

	virtual bool is_transmissive() const final override;

	virtual bool is_translucent() const override;

protected:

	math::float3 base_evaluate(math::pfloat3 wi, float& pdf) const;

	void diffuse_importance_sample(sampler::Sampler& sampler, bxdf::Result& result) const;
	void specular_importance_sample(sampler::Sampler& sampler, bxdf::Result& result) const;
	void pure_specular_importance_sample(sampler::Sampler& sampler, bxdf::Result& result) const;

	math::float3 diffuse_color_;
	math::float3 f0_;
	math::float3 emission_;

	float a2_;
	float metallic_;

	friend oren_nayar::Oren_nayar;
	friend ggx::Schlick_isotropic;
};

}}}
