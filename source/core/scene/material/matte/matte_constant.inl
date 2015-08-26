#pragma once

#include "matte_constant.hpp"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/differential.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace matte {

template<bool Two_sided>
Constant<Two_sided>::Constant(Generic_sample_cache<Sample>& cache,
							  std::shared_ptr<image::texture::Texture_2D> mask,
							  const math::float3& color,
							  float sqrt_roughness) :
	Matte(cache, mask), color_(color), sqrt_roughness_(sqrt_roughness) {}

template<bool Two_sided>
const Sample& Constant<Two_sided>::sample(const shape::Differential& dg, const math::float3& wo,
										  const image::texture::sampler::Sampler_2D& /*sampler*/,
										  uint32_t worker_id) {
	auto& sample = cache_.get(worker_id);

	sample.set_basis(dg.t, dg.b, dg.n, dg.geo_n, wo);
	sample.set(color_, sqrt_roughness_);

	return sample;
}

}}}

