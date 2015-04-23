#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample.hpp"
#include "scene/material/bxdf.hpp"

namespace scene { namespace material { namespace glass {

class Sample;

class BRDF : public BXDF<Sample> {
public:

	BRDF(const Sample& sample);

	virtual math::float3 evaluate(const math::float3& wi) const;
	virtual math::float3 importance_sample(sampler::Sampler& sampler, math::float3& wi, float& pdf) const;
};

class BTDF : public BXDF<Sample> {
public:

	BTDF(const Sample& sample);

	virtual math::float3 evaluate(const math::float3& wi) const;
	virtual math::float3 importance_sample(sampler::Sampler& sampler, math::float3& wi, float& pdf) const;
};

class Sample : public material::Sample {
public:

	Sample();

	virtual math::float3 evaluate(const math::float3& wi) const;

	virtual math::float3 sample_evaluate(sampler::Sampler& sampler, math::float3& wi, float& pdf) const;

	void set(const math::float3& color, float ior);

private:

	math::float3 color_;

	float ior_;

	BRDF brdf_;
	BTDF btdf_;

	friend BRDF;
	friend BTDF;
};

class Glass : public Material<Sample> {
public:

	Glass(Sample_cache<Sample>& cache);
};

}}}
