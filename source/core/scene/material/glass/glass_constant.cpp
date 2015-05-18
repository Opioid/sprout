#include "glass_constant.hpp"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/differential.hpp"
#include <algorithm>

// https://seblagarde.wordpress.com/2011/08/17/feeding-a-physical-based-lighting-mode/

namespace scene { namespace material { namespace glass {

Constant::Constant(Sample_cache<Sample>& cache, std::shared_ptr<image::Image> mask, const math::float3& color, float ior) :
	Glass(cache, mask), color_(color), ior_(std::max(ior, 1.0001f)) {
	float n = ior - 1.f;
	float d = ior + 1.f;
	f0_ = (n * n) / (d * d);
}

const Sample& Constant::sample(const shape::Differential& dg, const math::float3& wo,
							   const image::sampler::Sampler_2D& /*sampler*/, uint32_t worker_id) {
	auto& sample = cache_.get(worker_id);

	sample.set_basis(dg.t, dg.b, dg.n, dg.geo_n, wo);
	sample.set(color_, ior_, f0_);

	return sample;
}

}}}
