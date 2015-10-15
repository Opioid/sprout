#include "glass_constant.hpp"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/differential.hpp"
#include "base/math/vector.inl"
#include <algorithm>

// https://seblagarde.wordpress.com/2011/08/17/feeding-a-physical-based-lighting-mode/

namespace scene { namespace material { namespace glass {

float attenuation(float x, float d) {
	return x > 0.f ? 1.f / (x * d) : 0.f;
}

Constant::Constant(Generic_sample_cache<Sample>& cache, std::shared_ptr<image::texture::Texture_2D> mask,
				   const math::float3& color, float attenuation_distance, float ior) :
	Glass(cache, mask), color_(color),
	attenuation_(attenuation(color.x, attenuation_distance),
				 attenuation(color.y, attenuation_distance),
				 attenuation(color.z, attenuation_distance)),
	ior_(std::max(ior, 1.0001f)) {}

const Sample& Constant::sample(const shape::Differential& dg, const math::float3& wo,
							   const image::texture::sampler::Sampler_2D& /*sampler*/,
							   uint32_t worker_id) {
	auto& sample = cache_.get(worker_id);

	sample.set_basis(dg.t, dg.b, dg.n, dg.geo_n, wo);
	sample.set(color_, attenuation_, ior_);

	return sample;
}

}}}
