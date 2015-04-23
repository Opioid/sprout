#pragma once

#include "glass.hpp"

namespace scene { namespace material { namespace glass {

class Constant : public Glass {
public:

	Constant(Sample_cache<Sample>& cache, const math::float3& color, float ior);

	virtual const Sample& sample(const shape::Differential& dg, const math::float3& wo,
								 const image::sampler::Sampler_2D& sampler, uint32_t worker_id);

private:

	math::float3 color_;

	float ior_;
};

}}}
