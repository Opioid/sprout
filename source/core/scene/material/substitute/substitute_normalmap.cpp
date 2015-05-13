#include "substitute_normalmap.hpp"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/differential.hpp"
#include "image/texture/texture_2d.inl"
#include "image/texture/sampler/sampler_2d.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace substitute {

Normalmap::Normalmap(Sample_cache<Sample>& cache, std::shared_ptr<image::Image> mask, const math::float3& color,
					 std::shared_ptr<image::Image> normal, float roughness, float metallic) :
	Substitute(cache, mask), color_(color), normal_(normal), roughness_(roughness), metallic_(metallic) {}

const Sample& Normalmap::sample(const shape::Differential& dg, const math::float3& wo,
								const image::sampler::Sampler_2D& sampler, uint32_t worker_id) {
	auto& sample = cache_.get(worker_id);

	math::float3 nm = sampler.sample3(normal_, dg.uv);
	math::float3 n = math::normalized(dg.tangent_to_world(nm));

//	math::float3 t;
//	math::float3 b;
//	math::coordinate_system(n, t, b);

	sample.set_basis(dg.t, dg.b, n, wo);

	sample.set(color_, roughness_, metallic_);

	return sample;
}

}}}
