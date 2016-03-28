#pragma once

#include "scene/material/material_sample.hpp"
#include "scene/material/ggx/ggx.hpp"
#include "scene/material/oren_nayar/oren_nayar.hpp"

namespace scene { namespace material { namespace substitute {

class Sample : public material::Sample {
public:

	virtual math::float3 evaluate(math::pfloat3 wi, float& pdf) const final override;

	virtual math::float3 emission() const final override;

	virtual math::float3 attenuation() const final override;

	virtual float ior() const final override;

	virtual void sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const final override;

	virtual bool is_pure_emissive() const final override;

	virtual bool is_transmissive() const final override;

	virtual bool is_translucent() const final override;

	void set(const math::float3& color, const math::float3& emission,
			 float constant_f0, float roughness, float metallic);

private:

	math::float3 diffuse_color_;
	math::float3 f0_;
	math::float3 emission_;

	float a2_;
	float metallic_;

	oren_nayar::Oren_nayar<Sample> oren_nayar_;
	ggx::Schlick_isotropic<Sample> ggx_;

	friend oren_nayar::Oren_nayar<Sample>;
	friend ggx::Schlick_isotropic<Sample>;
};

}}}
