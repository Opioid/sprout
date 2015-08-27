#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample_cache.hpp"
#include "scene/material/material_sample.hpp"
#include "scene/material/bxdf.hpp"

namespace scene { namespace material { namespace glass {

class Sample;

class BRDF : public BxDF<Sample> {
public:

	BRDF(const Sample& sample);

	math::float3 evaluate(const math::float3& wi, float n_dot_wi) const;

	float pdf(const math::float3& wi, float n_dot_wi) const;

	float importance_sample(sampler::Sampler& sampler, BxDF_result& result) const;
};

class BTDF : public BxDF<Sample> {
public:

	BTDF(const Sample& sample);

	math::float3 evaluate(const math::float3& wi, float n_dot_wi) const;

	float pdf(const math::float3& wi, float n_dot_wi) const;

	float importance_sample(sampler::Sampler& sampler, BxDF_result& result) const;
};

class Sample : public material::Sample {
public:

	Sample();

	virtual math::float3 evaluate(const math::float3& wi, float& pdf) const final override;

	virtual math::float3 emission() const final override;

	virtual math::float3 attenuation() const final override;

	virtual void sample_evaluate(sampler::Sampler& sampler, BxDF_result& result) const final override;

	virtual bool is_pure_emissive() const final override;

	virtual bool is_translucent() const final override;

	void set(const math::float3& color, const math::float3& attenuation, float ior);

private:

	math::float3 color_;
	math::float3 attenuation_;
	float ior_;

	BRDF brdf_;
	BTDF btdf_;

	friend BRDF;
	friend BTDF;
};

class Glass : public Material<Generic_sample_cache<Sample>> {
public:

	Glass(Generic_sample_cache<Sample>& cache, std::shared_ptr<image::texture::Texture_2D> mask);

	virtual math::float3 sample_emission(math::float2 uv, const image::texture::sampler::Sampler_2D& sampler) const final override;

	virtual math::float3 average_emission() const final override;

	virtual const image::texture::Texture_2D* emission_map() const final override;

};

}}}
