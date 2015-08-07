#include "matte_colormap.hpp"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/differential.hpp"
#include "image/texture/sampler/sampler_2d.hpp"

namespace scene { namespace material { namespace matte {

Colormap::Colormap(Sample_cache<Sample>& cache,
				   std::shared_ptr<image::texture::Texture_2D> mask,
				   bool two_sided,
				   std::shared_ptr<image::texture::Texture_2D> color,
				   float sqrt_roughness) :
	Matte(cache, mask, two_sided), color_(color), sqrt_roughness_(sqrt_roughness) {}

const Sample& Colormap::sample(const shape::Differential& dg, const math::float3& wo,
							   const image::texture::sampler::Sampler_2D& sampler,
							   uint32_t worker_id) {
	auto& sample = cache_.get(worker_id);

	sample.set_basis(dg.t, dg.b, dg.n, dg.geo_n, wo, two_sided_);

	math::float3 color = sampler.sample_3(*color_, dg.uv);
	sample.set(color, sqrt_roughness_);

	return sample;
}

}}}

