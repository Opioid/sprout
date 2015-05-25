#pragma once

#include "matte.hpp"

namespace scene { namespace material { namespace matte {

class Constant : public Matte {
public:

	Constant(Sample_cache<Sample>& cache, std::shared_ptr<image::Image> mask, const math::float3& color);

	virtual const Sample& sample(const shape::Differential& dg, const math::float3& wo,
								 const image::sampler::Sampler_2D& sampler, uint32_t worker_id);

private:

	math::float3 color_;
};

}}}

