#pragma once

#include "substitute_normalmap.hpp"
#include "scene/material/material_sample.inl"
#include "scene/shape/geometry/differential.hpp"
#include "image/texture/sampler/sampler_2d.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace substitute {

template<bool Two_sided, bool Thin>
Normalmap<Two_sided, Thin>::Normalmap(
		Sample_cache& cache,
		std::shared_ptr<image::texture::Texture_2D> mask,
		const math::float3& color,
		std::shared_ptr<image::texture::Texture_2D> normal,
		float roughness,
		float metallic) :
	Substitute(cache, mask), color_(color), normal_(normal), roughness_(roughness), metallic_(metallic) {}

template<bool Two_sided, bool Thin>
const material::Sample& Normalmap<Two_sided, Thin>::sample(
		const shape::Differential& dg, const math::float3& wo,
		const image::texture::sampler::Sampler_2D& sampler,
		uint32_t worker_id) {
	auto& sample = cache_.get<Thin>(worker_id);

	math::float3 nm = sampler.sample_3(*normal_, dg.uv);
	math::float3 n = math::normalized(dg.tangent_to_world(nm));

	sample.template set_basis<Two_sided>(dg.t, dg.b, n, dg.geo_n, wo);

	sample.set(color_, roughness_, metallic_);

	return sample;
}

}}}
