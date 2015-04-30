#pragma once

#include "light_material.hpp"

namespace scene { namespace material { namespace light {

class Constant : public Light {
public:

	Constant(Sample_cache<Sample>& cache, const math::float3& emission);

	virtual const Sample& sample(const shape::Differential& dg, const math::float3& wo,
								 const image::sampler::Sampler_2D& sampler, uint32_t worker_id) final override;

	virtual math::float3 sample_emission() const final override;

private:

	math::float3 emission_;
};

}}}
