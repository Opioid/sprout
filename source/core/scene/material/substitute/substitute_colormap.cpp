#include "substitute_colormap.hpp"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/differential.hpp"
#include "image/texture/sampler/sampler_2d.hpp"

namespace scene { namespace material { namespace substitute {

Colormap::Colormap(Sample_cache<Sample>& cache, std::shared_ptr<image::Image> mask, std::shared_ptr<image::Image> color, float roughness, float metallic) :
	Substitute(cache, mask), color_(color), roughness_(roughness), metallic_(metallic) {}

const Sample& Colormap::sample(const shape::Differential& dg, const math::float3& wo,
							   const image::sampler::Sampler_2D& sampler, uint32_t worker_id) {
	auto& sample = cache_.get(worker_id);

	sample.set_basis(dg.t, dg.b, dg.n, wo);

	math::float3 color = sampler.sample3(color_, dg.uv);

	sample.set(color, roughness_, metallic_);

	return sample;
}

}}}
