#pragma once

#include "scene/material/material_sample.hpp"
#include "scene/material/ggx/ggx.hpp"
#include "scene/material/lambert/lambert.hpp"
#include "scene/material/oren_nayar/oren_nayar.hpp"

namespace scene { namespace material { namespace substitute {

class Sample_clearcoat;

class Clearcoat {
public:

	math::float3 evaluate(const Sample_clearcoat& sample, const math::float3& wi, float n_dot_wi) const;

	float pdf(const Sample_clearcoat& sample, const math::float3& wi, float n_dot_wi) const;

	float importance_sample(const Sample_clearcoat& sample, sampler::Sampler& sampler, bxdf::Result& result) const;

	float fresnel(const Sample_clearcoat& sample) const;
};

class Sample_clearcoat : public material::Sample {
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

	float ior_;

	Clearcoat clearcoat_;
	oren_nayar::Oren_nayar<Sample_clearcoat> oren_nayar_;
	ggx::Schlick_isotropic<Sample_clearcoat> ggx_;

	friend Clearcoat;
	friend oren_nayar::Oren_nayar<Sample_clearcoat>;
	friend ggx::Schlick_isotropic<Sample_clearcoat>;
};

}}}
