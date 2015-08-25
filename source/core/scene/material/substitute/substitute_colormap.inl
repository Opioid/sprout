#pragma once

#include "substitute_colormap.hpp"
#include "scene/material/material_sample.inl"
#include "scene/shape/geometry/differential.hpp"
#include "image/texture/sampler/sampler_2d.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace substitute {

template<bool Two_sided, bool Thin>
Colormap<Two_sided, Thin>::Colormap(
		Sample_cache& cache,
		std::shared_ptr<image::texture::Texture_2D> mask,
		std::shared_ptr<image::texture::Texture_2D> color,
		float roughness,
		float metallic) :
	Substitute(cache, mask), color_(color), roughness_(roughness), metallic_(metallic) {}

template<bool Two_sided, bool Thin>
const material::Sample& Colormap<Two_sided, Thin>::sample(
		const shape::Differential& dg, const math::float3& wo,
		const image::texture::sampler::Sampler_2D& sampler,
		uint32_t worker_id) {
	auto& sample = cache_.get<Thin>(worker_id);

	sample.template set_basis<Two_sided>(dg.t, dg.b, dg.n, dg.geo_n, wo);

	math::float3 color = sampler.sample_3(*color_, dg.uv);
	sample.set(color, roughness_, metallic_);

	return sample;
}

}}}
