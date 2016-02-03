#include "light_constant.hpp"
#include "light_material_sample.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/differential.hpp"
#include "base/color/color.inl"

namespace scene { namespace material { namespace light {

Constant::Constant(Generic_sample_cache<Sample>& cache,
				   std::shared_ptr<image::texture::Texture_2D> mask, bool two_sided,
				   const math::float3& emission) :
	Material(cache, mask, two_sided), emission_(emission) {}

const material::Sample& Constant::sample(const shape::Differential& dg, const math::float3& wo,
										 float /*time*/, float /*ior_i*/,
										 const image::texture::sampler::Sampler_2D& /*sampler*/,
										 uint32_t worker_id) {
	auto& sample = cache_.get(worker_id);

	sample.set_basis(dg.t, dg.b, dg.n, dg.geo_n, wo, two_sided_);
	sample.set(emission_);

	return sample;
}

math::float3 Constant::sample_emission(math::float2 /*uv*/, float /*time*/,
									   const image::texture::sampler::Sampler_2D& /*sampler*/) const {
	return emission_;
}

math::float3 Constant::average_emission() const {
	return emission_;
}

const image::texture::Texture_2D* Constant::emission_map() const {
	return nullptr;
}

}}}
