#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample.hpp"
#include "scene/material/lambert/lambert.hpp"

namespace scene { namespace material { namespace matte {

class Sample : public material::Sample {
public:

	Sample();

	virtual math::float3 evaluate(const math::float3& wi, float& pdf) const final override;

	virtual math::float3 emission() const final override;

	virtual math::float3 attenuation() const final override;

	virtual void sample_evaluate(sampler::Sampler& sampler, BxDF_result& result) const final override;

	void set(const math::float3& color);

private:

	math::float3 diffuse_color_;

	lambert::Lambert<Sample> lambert_;

	friend lambert::Lambert<Sample>;
};

class Matte : public Material<Sample> {
public:

	Matte(Sample_cache<Sample>& cache, std::shared_ptr<image::Image> mask);

	virtual math::float3 sample_emission() const override;

	virtual math::float3 average_emission() const override;
};

}}}

