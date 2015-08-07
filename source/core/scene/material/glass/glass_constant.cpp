#include "glass_constant.hpp"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/differential.hpp"
#include "base/math/vector.inl"
#include <algorithm>

// https://seblagarde.wordpress.com/2011/08/17/feeding-a-physical-based-lighting-mode/

namespace scene { namespace material { namespace glass {

Constant::Constant(Sample_cache<Sample>& cache, std::shared_ptr<image::texture::Texture_2D> mask,
				   const math::float3& color, float attenuation_distance, float ior) :
	Glass(cache, mask), color_(color),
	attenuation_(1.f / (color.x * attenuation_distance), 1.f / (color.y * attenuation_distance), 1.f / (color.z * attenuation_distance)),
	ior_(std::max(ior, 1.0001f)) {}

const Sample& Constant::sample(const shape::Differential& dg, const math::float3& wo,
							   const image::texture::sampler::Sampler_2D& /*sampler*/, uint32_t worker_id) {
	auto& sample = cache_.get(worker_id);

	sample.set_basis<false>(dg.t, dg.b, dg.n, dg.geo_n, wo);
	sample.set(color_, attenuation_, ior_);

	return sample;
}

}}}
