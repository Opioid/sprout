#include "display_material.hpp"
#include "display_sample.hpp"
#include "image/texture/texture_2d_adapter.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/material_sample_cache.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/shape/geometry/hitpoint.inl"
#include "base/spectrum/rgb.inl"
#include "base/math/math.hpp"
#include "base/math/distribution/distribution_2d.inl"

namespace scene { namespace material { namespace display {

Material::Material(Generic_sample_cache<Sample>& cache,
				   const Sampler_settings& sampler_settings, bool two_sided) :
	material::Typed_material<Generic_sample_cache<Sample>>(cache, sampler_settings, two_sided),
	average_emission_(float3(-1.f, -1.f, -1.f)) {}

const material::Sample& Material::sample(float3_p wo, const Renderstate& rs,
										 const Worker& worker, Sampler_filter filter) {
	auto& sample = cache_.get(worker.id());

	sample.set_basis(rs.geo_n, wo);

	sample.layer_.set_basis(rs.t, rs.b, rs.n);

	if (emission_map_.is_valid()) {
		auto& sampler = worker.sampler(sampler_key_, filter);

		float3 radiance = emission_map_.sample_3(sampler, rs.uv);
		sample.layer_.set(emission_factor_ * radiance, f0_, roughness_);
	} else {
		sample.layer_.set(emission_factor_ * emission_, f0_, roughness_);
	}

	return sample;
}

float3 Material::sample_radiance(float3_p /*wi*/, float2 uv,
								 float /*area*/, float /*time*/, const Worker& worker,
								 Sampler_filter filter) const {
	auto& sampler = worker.sampler(sampler_key_, filter);

	return emission_factor_ * emission_map_.sample_3(sampler, uv);
}

float3 Material::average_radiance(float /*area*/) const {
	return average_emission_;
}

bool Material::has_emission_map() const {
	return emission_map_.is_valid();
}

float2 Material::radiance_sample(float2 r2, float& pdf) const {
	float2 uv = distribution_.sample_continuous(r2, pdf);

	if (uv.y == 0.f) {
		pdf = 0.f;
	} else {
		float sin_theta = std::sin(uv.y * math::Pi);

		pdf *= total_weight_ / sin_theta;
	}

	return uv;
}

float Material::emission_pdf(float2 uv, const Worker& worker,
							 Sampler_filter filter) const {
	if (uv.y == 0.f) {
		return 0.f;
	}

	auto& sampler = worker.sampler(sampler_key_, filter);

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

		auto d = emission_map_.texture()->dimensions();
		std::vector<float> luminance(d.x * d.y);

		total_weight_ = 0.f;

		for (int32_t y = 0, l = 0; y < d.y; ++y) {
			float sin_theta = std::sin(((static_cast<float>(y) + 0.5f) /
										 static_cast<float>(d.y)) * math::Pi);

			for (int32_t x = 0; x < d.x; ++x, ++l) {
				float3 radiance = emission_factor_ * emission_map_.texture()->at_3(x, y);

				luminance[l] = spectrum::luminance(radiance);

				average_emission_ += sin_theta * radiance;

				total_weight_ += sin_theta;
			}
		}

		average_emission_ /= total_weight_;

		distribution_.init(luminance.data(), d);
	} else {
		average_emission_ = emission_factor_ * emission_map_.texture()->average_3();

		if (is_two_sided()) {
			average_emission_ *= 2.f;
		}
	}
}

void Material::set_emission_map(const Adapter_2D& emission_map) {
	emission_map_ = emission_map;
}

void Material::set_emission(float3_p radiance) {
	emission_ = radiance;
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
