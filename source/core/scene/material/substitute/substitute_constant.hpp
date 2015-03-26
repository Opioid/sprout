#pragma once

#include "substitute.hpp"

namespace scene { namespace material { namespace substitute {

class Constant : public Substitute {
public:

	Constant(Sample_cache<Sample>& cache, const math::float3& color);

	virtual const Sample& sample(const shape::Differential& dg, uint32_t worker_id);

private:

	math::float3 color_;
};

}}}
