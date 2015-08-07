#include "light_constant.hpp"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/differential.hpp"
#include "base/color/color.inl"

namespace scene { namespace material { namespace light {

Constant::Constant(Sample_cache<Sample>& cache, std::shared_ptr<image::texture::Texture_2D> mask, const math::float3& emission) :
	Light(cache, mask), emission_(emission) {}

const Sample& Constant::sample(const shape::Differential& dg, const math::float3& wo,
							   const image::texture::sampler::Sampler_2D& /*sampler*/, uint32_t worker_id) {
	auto& sample = cache_.get(worker_id);

	sample.set_basis<false>(dg.t, dg.b, dg.n, dg.geo_n, wo);
	sample.set(emission_);

	return sample;
}

math::float3 Constant::sample_emission(math::float2 /*uv*/, const image::texture::sampler::Sampler_2D& /*sampler*/) const {
	return emission_;
}

math::float3 Constant::average_emission() const {
	return emission_;
}

const image::texture::Texture_2D* Constant::emission_map() const {
	return nullptr;
}

}}}
