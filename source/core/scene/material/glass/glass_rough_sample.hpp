#pragma once

#include "scene/material/material_sample.hpp"

namespace scene { namespace material {

namespace bxdf { struct Result; }

namespace glass {

class Sample_rough;

class BRDF_rough {
public:

	float3 evaluate(const Sample_rough& sample, const float3& wi, float n_dot_wi) const;

	float pdf(const Sample_rough& sample, const float3& wi, float n_dot_wi) const;

	float importance_sample(const Sample_rough& sample, sampler::Sampler& sampler, bxdf::Result& result) const;
};

class BTDF_rough {
public:

	float3 evaluate(const Sample_rough& sample, const float3& wi, float n_dot_wi) const;

	float pdf(const Sample_rough& sample, const float3& wi, float n_dot_wi) const;

	float importance_sample(const Sample_rough& sample, sampler::Sampler& sampler, bxdf::Result& result) const;
};

class Sample_rough : public material::Sample {
public:

	virtual float3 evaluate(float3_p wi, float& pdf) const final override;

	virtual float3 radiance() const final override;

	virtual float3 attenuation() const final override;

	virtual float ior() const final override;

	virtual void sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const final override;

	virtual bool is_pure_emissive() const final override;

	virtual bool is_transmissive() const final override;

	virtual bool is_translucent() const final override;

	void set(const float3& color, float attenuation_distance, float ior, float ior_outside);

private:

	float3 color_;
	float3 attenuation_;
	float ior_;
	float ior_outside_;

	BRDF_rough brdf_;
	BTDF_rough btdf_;

	friend BRDF_rough;
	friend BTDF_rough;
};

}}}
