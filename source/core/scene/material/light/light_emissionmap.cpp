#include "light_emissionmap.hpp"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/differential.hpp"
#include "image/texture/sampler/sampler_2d.hpp"
#include "base/color/color.inl"
#include "base/math/math.hpp"
#include "base/math/distribution/distribution_2d.inl"

namespace scene { namespace material { namespace light {

Emissionmap::Emissionmap(Generic_sample_cache<Sample>& cache,
						 std::shared_ptr<image::texture::Texture_2D> mask,
						 std::shared_ptr<image::texture::Texture_2D> emission,
						 float emission_factor) :
	Light(cache, mask), emission_(emission), emission_factor_(emission_factor) {}

const Sample& Emissionmap::sample(const shape::Differential& dg, const math::float3& wo,
								  const image::texture::sampler::Sampler_2D& sampler, uint32_t worker_id) {
	auto& sample = cache_.get(worker_id);

	sample.set_basis(dg.t, dg.b, dg.n, dg.geo_n, wo);
	sample.set(emission_factor_ * sampler.sample_3(*emission_, dg.uv));

	return sample;
}

math::float3 Emissionmap::sample_emission(math::float2 uv,
										  const image::texture::sampler::Sampler_2D& sampler) const {
	return emission_factor_ * sampler.sample_3(*emission_, uv);
}

math::float3 Emissionmap::average_emission() const {
	return average_emission_;
}

const image::texture::Texture_2D* Emissionmap::emission_map() const {
	return emission_.get();
}

math::float2 Emissionmap::emission_importance_sample(math::float2 r2, float& pdf) const {
	math::float2 uv = distribution_.sample_continuous(r2, pdf);

	if (uv.y == 0.f) {
		pdf = 0.f;
	} else {
		float sin_theta = std::sin(uv.y * math::Pi);

		pdf *= total_weight_ / sin_theta;
	}

	return uv;
}

float Emissionmap::emission_pdf(math::float2 uv, const image::texture::sampler::Sampler_2D& sampler) const {
	if (uv.y == 0.f) {
		return 0.f;
	}

	float sin_theta = std::sin(uv.y * math::Pi);

	return distribution_.pdf(sampler.address(uv)) * (total_weight_ / sin_theta);
}

void Emissionmap::prepare_sampling(bool spherical) {
	if (spherical) {
		auto d = emission_->dimensions();
		std::vector<float> luminance(d.x * d.y);

		total_weight_ = 0.f;

		for (uint32_t y = 0, l = 0; y < d.y; ++y) {
			float sin_theta = std::sin(((static_cast<float>(y) + 0.5f) / static_cast<float>(d.y)) * math::Pi);

			for (uint32_t x = 0; x < d.x; ++x, ++l) {
				math::float3 emission = emission_factor_ * emission_->at_3(x, y);

				luminance[l] = color::luminance(emission);

				average_emission_ += sin_theta * emission;

				total_weight_ += sin_theta;
			}
		}

		average_emission_ /= total_weight_;

		distribution_.init(luminance.data(), d);
	} else {
		average_emission_ = emission_factor_ * emission_->average().xyz;
	}
}

}}}
