#pragma once

#include "substitute.hpp"

namespace scene { namespace material { namespace substitute {

class Constant : public Substitute {
public:

	Constant(Sample_cache<Sample>& cache, const math::float3& color, float roughness, float metallic);

	virtual const Sample& sample(const shape::Differential& dg, const math::float3& wo, uint32_t worker_id);

private:

	math::float3 color_;
	float roughness_;
	float metallic_;
};

}}}
