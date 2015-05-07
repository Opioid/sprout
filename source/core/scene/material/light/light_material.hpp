#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample.hpp"
#include "scene/material/bxdf.hpp"

namespace scene { namespace material { namespace light {

class Sample : public material::Sample {
public:

	Sample();

	virtual math::float3 evaluate(const math::float3& wi) const final override;

	virtual math::float3 emission() const final override;

	virtual void sample_evaluate(sampler::Sampler& sampler, Result& result) const final override;

	void set(const math::float3& emission);

private:

	math::float3 emission_;

};

class Light : public Material<Sample> {
public:

	Light(Sample_cache<Sample>& cache, std::shared_ptr<image::Image> mask);
};

}}}
