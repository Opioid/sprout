#include "substitute_constant.hpp"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/differential.hpp"

namespace scene { namespace material { namespace substitute {

Constant::Constant(Sample_cache<Sample>& cache, const math::float3& color) : Substitute(cache), color_(color) {}

const Sample& Constant::sample(const shape::Differential& dg, const math::float3& wo, uint32_t worker_id) {
	auto& sample = cache_.get(worker_id);

	sample.set_basis(dg.t, dg.b, dg.n, wo);
	sample.set(color_);

	return sample;
}

}}}
