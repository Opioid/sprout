#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample.hpp"
#include "scene/material/bxdf.hpp"

namespace scene { namespace material { namespace substitute {

class Sample;

class Lambert : public BXDF<Sample> {
public:

	Lambert(const Sample& sample);

	virtual math::float3 evaluate(const math::float3& wi) const;
	virtual math::float3 importance_sample(sampler::Sampler& sampler, math::float3& wi, float& pdf) const;
};

class GGX : public BXDF<Sample> {
public:

	GGX(const Sample& sample);

	virtual math::float3 evaluate(const math::float3& wi) const;
	virtual math::float3 importance_sample(sampler::Sampler& sampler, math::float3& wi, float& pdf) const;
};

class Sample : public material::Sample {
public:

	Sample();

	virtual math::float3 evaluate(const math::float3& wi) const;

	virtual math::float3 emission() const;

	virtual void sample_evaluate(sampler::Sampler& sampler, Result& result) const;

	void set(const math::float3& color, float roughness, float metallic);

private:

	math::float3 diffuse_color_;
	math::float3 f0_;

	float a2_;

	float metallic_;

	Lambert lambert_;
	GGX ggx_;

	friend Lambert;
	friend GGX;
};

class Substitute : public Material<Sample> {
public:

	Substitute(Sample_cache<Sample>& cache);

	virtual math::float3 sample_emission() const;
};

}}}
