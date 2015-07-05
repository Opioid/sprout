#include "light_emissionmap.hpp"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/differential.hpp"
#include "image/texture/texture_2d.inl"
#include "image/texture/sampler/sampler_2d.hpp"

namespace scene { namespace material { namespace light {

Emissionmap::Emissionmap(Sample_cache<Sample>& cache, std::shared_ptr<image::Image> mask, std::shared_ptr<image::Image> emission) :
	Light(cache, mask), emission_(emission) {}

const Sample& Emissionmap::sample(const shape::Differential& dg, const math::float3& wo,
								  const image::sampler::Sampler_2D& sampler, uint32_t worker_id) {
	auto& sample = cache_.get(worker_id);

	sample.set_basis(dg.t, dg.b, dg.n, dg.geo_n, wo);
	sample.set(sampler.sample3(emission_, dg.uv));

	return sample;
}

math::float3 Emissionmap::sample_emission(math::float2 uv, const image::sampler::Sampler_2D& sampler) const {
	return sampler.sample3(emission_, uv);
}

math::float3 Emissionmap::average_emission() const {
	return emission_.image()->average().xyz;
}

const image::Texture_2D* Emissionmap::emission_map() const {
	return &emission_;
}

}}}
