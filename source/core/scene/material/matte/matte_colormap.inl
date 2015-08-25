#pragma once

#include "matte_colormap.hpp"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/differential.hpp"
#include "image/texture/sampler/sampler_2d.hpp"

namespace scene { namespace material { namespace matte {

template<bool Two_sided>
Colormap<Two_sided>::Colormap(Generic_sample_cache<Sample>& cache,
							  std::shared_ptr<image::texture::Texture_2D> mask,
							  std::shared_ptr<image::texture::Texture_2D> color,
							  float sqrt_roughness) :
	Matte(cache, mask), color_(color), sqrt_roughness_(sqrt_roughness) {}

template<bool Two_sided>
const Sample& Colormap<Two_sided>::sample(const shape::Differential& dg, const math::float3& wo,
										  const image::texture::sampler::Sampler_2D& sampler,
										  uint32_t worker_id) {
	auto& sample = cache_.get(worker_id);

	sample.set_basis<Two_sided>(dg.t, dg.b, dg.n, dg.geo_n, wo);

	math::float3 color = sampler.sample_3(*color_, dg.uv);
	sample.set(color, sqrt_roughness_);

	return sample;
}

}}}

