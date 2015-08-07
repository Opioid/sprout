#include "substitute_colormap_surfacemap.hpp"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/differential.hpp"
#include "image/texture/sampler/sampler_2d.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace substitute {

Colormap_surfacemap::Colormap_surfacemap(Sample_cache<Sample>& cache,
										 std::shared_ptr<image::texture::Texture_2D> mask,
										 bool two_sided,
										 std::shared_ptr<image::texture::Texture_2D> color,
										 std::shared_ptr<image::texture::Texture_2D> surface) :
	Substitute(cache, mask, two_sided), color_(color), surface_(surface) {}

const Sample& Colormap_surfacemap::sample(const shape::Differential& dg, const math::float3& wo,
										  const image::texture::sampler::Sampler_2D& sampler,
										  uint32_t worker_id) {
	auto& sample = cache_.get(worker_id);

	sample.set_basis(dg.t, dg.b, dg.n, dg.geo_n, wo, two_sided_);

	math::float3 color   = sampler.sample_3(*color_, dg.uv);
	math::float2 surface = sampler.sample_2(*surface_, dg.uv);

	sample.set(color, surface.x, surface.y);

	return sample;
}

}}}
