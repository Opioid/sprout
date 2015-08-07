#include "matte_constant.hpp"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/differential.hpp"

namespace scene { namespace material { namespace matte {

Constant::Constant(Sample_cache<Sample>& cache,
				   std::shared_ptr<image::texture::Texture_2D> mask,
				   bool two_sided,
				   const math::float3& color,
				   float sqrt_roughness) :
	Matte(cache, mask, two_sided), color_(color), sqrt_roughness_(sqrt_roughness) {}

const Sample& Constant::sample(const shape::Differential& dg, const math::float3& wo,
							   const image::texture::sampler::Sampler_2D& /*sampler*/, uint32_t worker_id) {
	auto& sample = cache_.get(worker_id);

	sample.set_basis(dg.t, dg.b, dg.n, dg.geo_n, wo, two_sided_);
	sample.set(color_, sqrt_roughness_);

	return sample;
}

}}}

