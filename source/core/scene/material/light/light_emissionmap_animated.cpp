#include "light_emissionmap_animated.hpp"
#include "light_material_sample.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/differential.hpp"
#include "image/texture/sampler/sampler_2d.hpp"
#include "base/color/color.inl"
#include "base/math/math.hpp"
#include "base/math/distribution/distribution_2d.inl"

namespace scene { namespace material { namespace light {

Emissionmap_animated::Emissionmap_animated(Generic_sample_cache<Sample>& cache,
										   std::shared_ptr<image::texture::Texture_2D> mask, bool two_sided,
										   std::shared_ptr<image::texture::Texture_2D> emission_map,
										   float emission_factor, float animation_duration) :
	Material(cache, mask, two_sided), emission_map_(emission_map), emission_factor_(emission_factor),
	average_emissions_(emission_map->num_elements()),
	frame_length_(animation_duration / static_cast<float>(emission_map_->num_elements())),
	element_(0) {
	for (auto& ae : average_emissions_) {
		ae = math::vec3(-1.f, -1.f, -1.f);
	}
}

void Emissionmap_animated::tick(float absolute_time, float /*time_slice*/) {
	element_ = static_cast<int32_t>(absolute_time / frame_length_) % emission_map_->num_elements();
}

const material::Sample& Emissionmap_animated::sample(const shape::Differential& dg, const math::vec3& wo,
													 float /*time*/, float /*ior_i*/,
													 const image::texture::sampler::Sampler_2D& sampler,
													 uint32_t worker_id) {
	auto& sample = cache_.get(worker_id);

	sample.set_basis(dg.t, dg.b, dg.n, dg.geo_n, wo, two_sided_);

	math::vec3 emission = sampler.sample_3(*emission_map_, dg.uv, element_);
	sample.set(emission_factor_ * emission);

	return sample;
}

math::vec3 Emissionmap_animated::sample_emission(math::float2 uv, float /*time*/,
												   const image::texture::sampler::Sampler_2D& sampler) const {
	return emission_factor_ * sampler.sample_3(*emission_map_, uv, element_);
}

math::vec3 Emissionmap_animated::average_emission() const {
	return average_emissions_[element_];
}

bool Emissionmap_animated::has_emission_map() const {
	return nullptr != emission_map_;
}

math::float2 Emissionmap_animated::emission_importance_sample(math::float2 r2, float& pdf) const {
	math::float2 uv = distribution_.sample_continuous(r2, pdf);

	if (uv.y == 0.f) {
		pdf = 0.f;
	} else {
		float sin_theta = std::sin(uv.y * math::Pi);

		pdf *= total_weight_ / sin_theta;
	}

	return uv;
}

float Emissionmap_animated::emission_pdf(math::float2 uv, const image::texture::sampler::Sampler_2D& sampler) const {
	if (uv.y == 0.f) {
		return 0.f;
	}

	float sin_theta = std::sin(uv.y * math::Pi);

	return distribution_.pdf(sampler.address(uv)) * (total_weight_ / sin_theta);
}

float Emissionmap_animated::opacity(math::float2 uv, float /*time*/,
									const image::texture::sampler::Sampler_2D& sampler) const {
	if (mask_) {
		return sampler.sample_1(*mask_, uv, element_);
	} else {
		return 1.f;
	}
}

void Emissionmap_animated::prepare_sampling(bool spherical) {
	if (average_emissions_[element_].x >= 0.f) {
		// Hacky way to check whether prepare_sampling has been called before
		// average_emission_ is initialized with negative values...
		return;
	}

	if (spherical) {
	/*	average_emission_ = math::vec3::identity;

		auto d = emission_->dimensions();
		std::vector<float> luminance(d.x * d.y);

		total_weight_ = 0.f;

		for (int32_t y = 0, l = 0; y < d.y; ++y) {
			float sin_theta = std::sin(((static_cast<float>(y) + 0.5f) / static_cast<float>(d.y)) * math::Pi);

			for (int32_t x = 0; x < d.x; ++x, ++l) {
				math::vec3 emission = emission_factor_ * emission_->at_3(x, y);

				luminance[l] = color::luminance(emission);

				average_emission_ += sin_theta * emission;

				total_weight_ += sin_theta;
			}
		}

		average_emission_ /= total_weight_;

		distribution_.init(luminance.data(), d);*/
	} else {
		average_emissions_[element_] = emission_factor_ * emission_map_->average_3(element_);

		if (is_two_sided()) {
			average_emissions_[element_] *= 2.f;
		}
	}
}

bool Emissionmap_animated::is_animated() const {
	return true;
}

}}}
