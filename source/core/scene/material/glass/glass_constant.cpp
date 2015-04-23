#include "glass_constant.hpp"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/differential.hpp"

namespace scene { namespace material { namespace glass {

Constant::Constant(Sample_cache<Sample>& cache, const math::float3& color, float ior) :
	Glass(cache), color_(color), ior_(std::max(ior, 1.0001f)) {}

const Sample& Constant::sample(const shape::Differential& dg, const math::float3& wo,
							   const image::sampler::Sampler_2D& sampler, uint32_t worker_id) {
	auto& sample = cache_.get(worker_id);

	sample.set_basis(dg.t, dg.b, dg.n, wo);
	sample.set(color_, ior_);

	return sample;
}

}}}
