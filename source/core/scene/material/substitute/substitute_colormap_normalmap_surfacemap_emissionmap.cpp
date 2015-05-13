#include "substitute_colormap_normalmap_surfacemap_emissionmap.hpp"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/differential.hpp"
#include "image/texture/texture_2d.inl"
#include "image/texture/sampler/sampler_2d.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace substitute {

Colormap_normalmap_surfacemap_emissionmap::Colormap_normalmap_surfacemap_emissionmap(Sample_cache<Sample>& cache, std::shared_ptr<image::Image> mask,
																					 std::shared_ptr<image::Image> color,
																					 std::shared_ptr<image::Image> normal,
																					 std::shared_ptr<image::Image> surface,
																					 std::shared_ptr<image::Image> emission,
																					 float emission_factor, float metallic) :
	Substitute(cache, mask), color_(color), normal_(normal), surface_(surface), emission_(emission),
	emission_factor_(emission_factor), metallic_(metallic) {}

const Sample& Colormap_normalmap_surfacemap_emissionmap::sample(const shape::Differential& dg, const math::float3& wo,
																const image::sampler::Sampler_2D& sampler, uint32_t worker_id) {
	auto& sample = cache_.get(worker_id);

	math::float3 nm = sampler.sample3(normal_, dg.uv);
	math::float3 n = math::normalized(dg.tangent_to_world(nm));

//	math::float3 t;
//	math::float3 b;
//	math::coordinate_system(n, t, b);

	sample.set_basis(dg.t, dg.b, n, wo);

	math::float3 color    = sampler.sample3(color_, dg.uv);
	float roughness       = sampler.sample3(surface_, dg.uv).x;
	math::float3 emission = emission_factor_ * sampler.sample3(emission_, dg.uv);

	sample.set(color, emission, roughness, metallic_);

	return sample;
}

math::float3 Colormap_normalmap_surfacemap_emissionmap::average_emission() const {
	return emission_factor_ * emission_.image()->average().xyz;
}

}}}
