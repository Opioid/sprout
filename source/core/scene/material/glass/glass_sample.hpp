#pragma once

#include "scene/material/material_sample.hpp"

namespace scene { namespace material {

namespace bxdf { struct Result; }

namespace glass {

class Sample;

class BRDF {
public:

	math::float3 evaluate(const Sample& sample, const math::float3& wi, float n_dot_wi) const;

	float pdf(const Sample& sample, const math::float3& wi, float n_dot_wi) const;

	float importance_sample(const Sample& sample, sampler::Sampler& sampler, bxdf::Result& result) const;
};

class BTDF {
public:

	math::float3 evaluate(const Sample& sample, const math::float3& wi, float n_dot_wi) const;

	float pdf(const Sample& sample, const math::float3& wi, float n_dot_wi) const;

	float importance_sample(const Sample& sample, sampler::Sampler& sampler, bxdf::Result& result) const;
};

class Sample : public material::Sample {
public:

	virtual math::float3 evaluate(const math::float3& wi, float& pdf) const final override;

	virtual math::float3 emission() const final override;

	virtual math::float3 attenuation() const final override;

	virtual void sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const final override;

	virtual bool is_pure_emissive() const final override;

	virtual bool is_translucent() const final override;

	void set(const math::float3& color, float attenuation_distance, float ior, float ior_outside);

private:

	math::float3 color_;
	math::float3 attenuation_;
	float ior_;
	float ior_outside_;

	BRDF brdf_;
	BTDF btdf_;

	friend BRDF;
	friend BTDF;
};

}}}
