#pragma once

#include "scene/material/material_sample.hpp"
#include "scene/material/ggx/ggx.hpp"

namespace scene { namespace material { namespace metal {

class Sample_iso : public material::Sample {
public:

	virtual math::float3 evaluate(const math::float3& wi, float& pdf) const final override;

	virtual math::float3 emission() const final override;

	virtual math::float3 attenuation() const final override;

	virtual void sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const final override;

	virtual bool is_pure_emissive() const final override;

	virtual bool is_translucent() const final override;

	void set(const math::float3& ior, const math::float3& absorption, float roughness);

private:

	math::float3 ior_;
	math::float3 absorption_;

	float a2_;

	ggx::Isotropic_conductor<Sample_iso> ggx_;

	friend ggx::Isotropic_conductor<Sample_iso>;
};

class Sample_aniso : public material::Sample {
public:

	virtual math::float3 evaluate(const math::float3& wi, float& pdf) const final override;

	virtual math::float3 emission() const final override;

	virtual math::float3 attenuation() const final override;

	virtual void sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const final override;

	virtual bool is_pure_emissive() const final override;

	virtual bool is_translucent() const final override;

	void set(const math::float3& ior, const math::float3& absorption, math::float2 roughness);

private:

	math::float3 ior_;
	math::float3 absorption_;
	math::float2 a_;
	math::float2 a2_;
	float axy_;

	ggx::Anisotropic_conductor<Sample_aniso> ggx_;

	friend ggx::Anisotropic_conductor<Sample_aniso>;
};

}}}

