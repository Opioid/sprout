#include "display_material.hpp"
#include "display_sample.hpp"
#include "image/texture/sampler/sampler_2d.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/material_sample_cache.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/shape/geometry/differential.hpp"
#include "base/color/color.inl"
#include "base/math/math.hpp"
#include "base/math/distribution/distribution_2d.inl"

namespace scene { namespace material { namespace display {

Material::Material(Generic_sample_cache<Sample>& cache,
				   std::shared_ptr<image::texture::Texture_2D> mask, bool two_sided) :
	material::Material<Generic_sample_cache<Sample>>(cache, mask, two_sided),
	average_emission_(math::float3(-1.f, -1.f, -1.f)) {}

const material::Sample& Material::sample(const shape::Differential& dg, const math::float3& wo,
										 float /*time*/, float /*ior_i*/,
										 const image::texture::sampler::Sampler_2D& sampler, uint32_t worker_id) {
	auto& sample = cache_.get(worker_id);

	sample.set_basis(dg.t, dg.b, dg.n, dg.geo_n, wo, two_sided_);

	if (emission_map_) {
		math::float3 emission = sampler.sample_3(*emission_map_, dg.uv);
		sample.set(emission_factor_ * emission, f0_, roughness_);
	} else {
		sample.set(emission_factor_ * emission_, f0_, roughness_);
	}

	return sample;
}

math::float3 Material::sample_emission(math::float2 uv, float /*time*/,
										  const image::texture::sampler::Sampler_2D& sampler) const {
	return emission_factor_ * sampler.sample_3(*emission_map_, uv);
}

math::float3 Material::average_emission() const {
	return average_emission_;
}

bool Material::has_emission_map() const {
	return nullptr != emission_map_;
}

math::float2 Material::emission_importance_sample(math::float2 r2, float& pdf) const {
	math::float2 uv = distribution_.sample_continuous(r2, pdf);

	if (uv.y == 0.f) {
		pdf = 0.f;
	} else {
		float sin_theta = std::sin(uv.y * math::Pi);

		pdf *= total_weight_ / sin_theta;
	}

	return uv;
}

float Material::emission_pdf(math::float2 uv, const image::texture::sampler::Sampler_2D& sampler) const {
	if (uv.y == 0.f) {
		return 0.f;
	}

	float sin_theta = std::sin(uv.y * math::Pi);

	return distribution_.pdf(sampler.address(uv)) * (total_weight_ / sin_theta);
}

void Material::prepare_sampling(bool spherical) {
	if (average_emission_.x >= 0.f) {
		// Hacky way to check whether prepare_sampling has been called before
		// average_emission_ is initialized with negative values...
		return;
	}

	if (spherical) {
		average_emission_ = math::float3_identity;

		auto d = emission_map_->dimensions();
		std::vector<float> luminance(d.x * d.y);

		total_weight_ = 0.f;

		for (int32_t y = 0, l = 0; y < d.y; ++y) {
			float sin_theta = std::sin(((static_cast<float>(y) + 0.5f) / static_cast<float>(d.y)) * math::Pi);

			for (int32_t x = 0; x < d.x; ++x, ++l) {
				math::float3 emission = emission_factor_ * emission_map_->at_3(x, y);

				luminance[l] = color::luminance(emission);

				average_emission_ += sin_theta * emission;

				total_weight_ += sin_theta;
			}
		}

		average_emission_ /= total_weight_;

		distribution_.init(luminance.data(), d);
	} else {
		average_emission_ = emission_factor_ * emission_map_->average_3();

		if (is_two_sided()) {
			average_emission_ *= 2.f;
		}
	}
}

void Material::set_emission_map(std::shared_ptr<image::texture::Texture_2D> emission_map) {
	emission_map_ = emission_map;
}

void Material::set_emission(const math::float3& emission) {
	emission_ = emission;
}

void Material::set_emission_factor(float emission_factor) {
	emission_factor_ = emission_factor;
}

void Material::set_roughness(float roughness) {
	roughness_ = roughness;
}

void Material::set_ior(float ior) {
	f0_ = fresnel::schlick_f0(1.f, ior);
}

}}}
