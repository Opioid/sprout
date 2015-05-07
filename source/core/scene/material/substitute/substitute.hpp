#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample.hpp"
#include "scene/material/bxdf.hpp"

namespace scene { namespace material { namespace substitute {

class Sample;

class Lambert : public BXDF<Sample> {
public:

	Lambert(const Sample& sample);

	virtual math::float3 evaluate(const math::float3& wi) const final override;
	virtual math::float3 importance_sample(sampler::Sampler& sampler, math::float3& wi, float& pdf) const final override;
};

class GGX : public BXDF<Sample> {
public:

	GGX(const Sample& sample);

	virtual math::float3 evaluate(const math::float3& wi) const final override;
	virtual math::float3 importance_sample(sampler::Sampler& sampler, math::float3& wi, float& pdf) const final override;
};

class Sample : public material::Sample {
public:

	Sample();

	virtual math::float3 evaluate(const math::float3& wi) const final override;

	virtual math::float3 emission() const final override;

	virtual void sample_evaluate(sampler::Sampler& sampler, Result& result) const final override;

	void set(const math::float3& color, float roughness, float metallic);
	void set(const math::float3& color, const math::float3& emission, float roughness, float metallic);

private:

	math::float3 diffuse_color_;
	math::float3 f0_;
	math::float3 emission_;

	float a2_;

	float metallic_;

	Lambert lambert_;
	GGX ggx_;

	friend Lambert;
	friend GGX;
};

class Substitute : public Material<Sample> {
public:

	Substitute(Sample_cache<Sample>& cache, std::shared_ptr<image::Image> mask);

	virtual math::float3 sample_emission() const;
};

}}}
