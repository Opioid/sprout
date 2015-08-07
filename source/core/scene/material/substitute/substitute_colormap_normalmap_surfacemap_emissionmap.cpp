#include "substitute_colormap_normalmap_surfacemap_emissionmap.hpp"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/differential.hpp"
#include "image/texture/sampler/sampler_2d.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace substitute {

Colormap_normalmap_surfacemap_emissionmap::Colormap_normalmap_surfacemap_emissionmap(
		Sample_cache<Sample>& cache,
		std::shared_ptr<image::texture::Texture_2D> mask,
		bool two_sided,
		std::shared_ptr<image::texture::Texture_2D> color,
		std::shared_ptr<image::texture::Texture_2D> normal,
		std::shared_ptr<image::texture::Texture_2D> surface,
		std::shared_ptr<image::texture::Texture_2D> emission,
		float emission_factor) :
	Substitute(cache, mask, two_sided), color_(color), normal_(normal), surface_(surface), emission_(emission),
	emission_factor_(emission_factor) {}

const Sample& Colormap_normalmap_surfacemap_emissionmap::sample(const shape::Differential& dg, const math::float3& wo,
																const image::texture::sampler::Sampler_2D& sampler, uint32_t worker_id) {
	auto& sample = cache_.get(worker_id);

	math::float3 nm = sampler.sample_3(*normal_, dg.uv);
	math::float3 n = math::normalized(dg.tangent_to_world(nm));

	sample.set_basis(dg.t, dg.b, n, dg.geo_n, wo, two_sided_);

	math::float3 color    = sampler.sample_3(*color_, dg.uv);
	math::float2 surface  = sampler.sample_2(*surface_, dg.uv);
	math::float3 emission = emission_factor_ * sampler.sample_3(*emission_, dg.uv);

	sample.set(color, emission, surface.x, surface.y);

	return sample;
}

math::float3 Colormap_normalmap_surfacemap_emissionmap::average_emission() const {
	return emission_factor_ * emission_->average().xyz;
}

}}}
