#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample.hpp"
#include "scene/material/bxdf.hpp"

namespace scene { namespace material { namespace glass {

class Sample;

class BRDF : public BxDF<Sample> {
public:

	BRDF(const Sample& sample);

	virtual math::float3 evaluate(const math::float3& wi) const;
	virtual void importance_sample(sampler::Sampler& sampler, BxDF_result& result) const;
};

class BTDF : public BxDF<Sample> {
public:

	BTDF(const Sample& sample);

	virtual math::float3 evaluate(const math::float3& wi) const;
	virtual void importance_sample(sampler::Sampler& sampler, BxDF_result& result) const;
};

class Sample : public material::Sample {
public:

	Sample();

	virtual math::float3 evaluate(const math::float3& wi) const final override;

	virtual math::float3 emission() const final override;

	virtual math::float3 attenuation() const final override;

	virtual void sample_evaluate(sampler::Sampler& sampler, BxDF_result& result) const final override;

	void set(const math::float3& color, const math::float3& attenuation, float ior, float f0);

private:

	math::float3 color_;
	math::float3 attenuation_;
	float ior_;
	float f0_;

	BRDF brdf_;
	BTDF btdf_;

	friend BRDF;
	friend BTDF;
};

class Glass : public Material<Sample> {
public:

	Glass(Sample_cache<Sample>& cache, std::shared_ptr<image::Image> mask);

	virtual math::float3 sample_emission() const final override;

	virtual math::float3 average_emission() const final override;
};

}}}
