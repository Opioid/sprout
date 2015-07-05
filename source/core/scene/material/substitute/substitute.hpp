#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/lambert/lambert.hpp"

namespace scene { namespace material { namespace substitute {

class Sample;

class GGX : public BxDF<Sample> {
public:

	GGX(const Sample& sample);

	virtual math::float3 evaluate(const math::float3& wi, float n_dot_wi) const final override;

	virtual float pdf(const math::float3& wi, float n_dot_wi) const final override;

	virtual float importance_sample(sampler::Sampler& sampler, BxDF_result& result) const final override;
};

class Sample : public material::Sample {
public:

	Sample();

	virtual math::float3 evaluate(const math::float3& wi, float& pdf) const final override;

	virtual math::float3 emission() const final override;

	virtual math::float3 attenuation() const final override;

	virtual void sample_evaluate(sampler::Sampler& sampler, BxDF_result& result) const final override;

	virtual bool is_pure_emissive() const final override;

	void set(const math::float3& color, float roughness, float metallic);
	void set(const math::float3& color, const math::float3& emission, float roughness, float metallic);

private:

	math::float3 diffuse_color_;
	math::float3 f0_;
	math::float3 emission_;

	float a2_;

	float metallic_;

	lambert::Lambert<Sample> lambert_;
	GGX ggx_;

	friend lambert::Lambert<Sample>;
	friend GGX;
};

class Substitute : public Material<Sample> {
public:

	Substitute(Sample_cache<Sample>& cache, std::shared_ptr<image::Image> mask);

	virtual math::float3 sample_emission(math::float2 uv, const image::sampler::Sampler_2D& sampler) const override;

	virtual math::float3 average_emission() const override;

	virtual const image::Texture_2D* emission_map() const override;
};

}}}
