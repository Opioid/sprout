#pragma once

#include "scene/material/material_sample.hpp"
#include "scene/material/ggx/ggx.hpp"

namespace scene { namespace material { namespace metal {

class Sample_isotropic : public material::Sample {

public:

	virtual float3 evaluate(float3_p wi, float& pdf) const final override;

	virtual float3 radiance() const final override;

	virtual float3 attenuation() const final override;

	virtual float ior() const final override;

	virtual void sample_evaluate(sampler::Sampler& sampler,
								 bxdf::Result& result) const final override;

	virtual bool is_pure_emissive() const final override;

	virtual bool is_transmissive() const final override;

	virtual bool is_translucent() const final override;

	void set(float3_p ior, float3_p absorption, float roughness);

private:

	float3 ior_;
	float3 absorption_;

	float a2_;

	friend ggx::Isotropic;
};

class Sample_anisotropic : public material::Sample {

public:

	virtual float3 evaluate(float3_p wi, float& pdf) const final override;

	virtual float3 radiance() const final override;

	virtual float3 attenuation() const final override;

	virtual float ior() const final override;

	virtual void sample_evaluate(sampler::Sampler& sampler,
								 bxdf::Result& result) const final override;

	virtual bool is_pure_emissive() const final override;

	virtual bool is_transmissive() const final override;

	virtual bool is_translucent() const final override;

	void set(float3_p ior, float3_p absorption, float2 roughness);

private:

	float3 ior_;
	float3 absorption_;
	float2 a_;
	float2 a2_;
	float axy_;

	friend ggx::Anisotropic;
};

}}}

