#include "substitute_colormap_normalmap_surfacemap.hpp"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/differential.hpp"
#include "image/texture/texture_2d.inl"
#include "image/texture/sampler/sampler_2d.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace substitute {

Colormap_normalmap_surfacemap::Colormap_normalmap_surfacemap(Sample_cache<Sample>& cache, std::shared_ptr<image::Image> mask,
															 std::shared_ptr<image::Image> color,
															 std::shared_ptr<image::Image> normal,
															 std::shared_ptr<image::Image> surface,
															 float metallic) :
	Substitute(cache, mask), color_(color), normal_(normal), surface_(surface), metallic_(metallic) {}

const Sample& Colormap_normalmap_surfacemap::sample(const shape::Differential& dg, const math::float3& wo,
													const image::sampler::Sampler_2D& sampler, uint32_t worker_id) {
	auto& sample = cache_.get(worker_id);

	math::float3 nm = sampler.sample3(normal_, dg.uv);
	math::float3 n = math::normalized(dg.tangent_to_world(nm));

//	math::float3 t;
//	math::float3 b;
//	math::coordinate_system(n, t, b);

	sample.set_basis(dg.t, dg.b, n, dg.geo_n, wo);

	math::float3 color = sampler.sample3(color_, dg.uv);
	float roughness    = sampler.sample3(surface_, dg.uv).x;

	sample.set(color, roughness, metallic_);

	return sample;
}

}}}
