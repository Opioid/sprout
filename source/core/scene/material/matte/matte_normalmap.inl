#pragma once

#include "matte_normalmap.hpp"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/differential.hpp"
#include "image/texture/sampler/sampler_2d.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace matte {

template<bool Two_sided>
Normalmap<Two_sided>::Normalmap(Generic_sample_cache<Sample>& cache,
								std::shared_ptr<image::texture::Texture_2D> mask,
								const math::float3& color,
								std::shared_ptr<image::texture::Texture_2D> normal,
								float sqrt_roughness) :
	Matte(cache, mask), color_(color), normal_(normal), sqrt_roughness_(sqrt_roughness) {}

template<bool Two_sided>
const Sample& Normalmap<Two_sided>::sample(const shape::Differential& dg, const math::float3& wo,
										   const image::texture::sampler::Sampler_2D& sampler,
										   uint32_t worker_id) {
	auto& sample = cache_.get(worker_id);

	math::float3 nm = sampler.sample_3(*normal_, dg.uv);
	math::float3 n = math::normalized(dg.tangent_to_world(nm));

	sample.set_basis(dg.t, dg.b, n, dg.geo_n, wo);

	sample.set(color_, sqrt_roughness_);

	return sample;
}

}}}

