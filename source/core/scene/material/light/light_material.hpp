#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample.hpp"
#include "scene/material/bxdf.hpp"

namespace scene { namespace material { namespace light {

class Sample : public material::Sample {
public:

	Sample();

	virtual math::float3 evaluate(const math::float3& wi) const;

	virtual void sample_evaluate(sampler::Sampler& sampler, Result& result) const;

	void set(const math::float3& emission);

private:

	math::float3 emission_;

};

class Light : public Material<Sample> {
public:

	Light(Sample_cache<Sample>& cache);
};

}}}
