#include "light_emissionmap.hpp"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/differential.hpp"
#include "image/texture/texture_2d.inl"
#include "image/texture/sampler/sampler_2d.hpp"
#include "base/color/color.inl"

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

math::float2 Emissionmap::emission_importance_sample(math::float2 r2, float& pdf) const {
	math::float2 uv = distribution_.sample_continuous(r2, pdf);
	pdf *= num_pixels_;
	return uv;
}

float Emissionmap::emission_pdf(math::float2 uv, const image::sampler::Sampler_2D& sampler) const {
	return distribution_.pdf(sampler.address(uv)) * num_pixels_;
}

void Emissionmap::prepare_sampling() {
	std::vector<float> luminance;
	auto d = emission_.dimensions();
	luminance.resize(d.x * d.y);

	auto image = emission_.image();
	for (uint32_t i = 0, len = static_cast<uint32_t>(luminance.size()); i < len; ++i) {
		luminance[i] = color::luminance(image->at3(i));
	}

	distribution_.init(luminance.data(), d);

	num_pixels_ = static_cast<float>(d.x * d.y);
}

}}}
