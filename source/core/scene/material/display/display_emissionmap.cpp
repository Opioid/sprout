#include "display_emissionmap.hpp"
#include "display_sample.hpp"
#include "image/texture/texture_adapter.inl"
#include "scene/scene_renderstate.inl"
#include "scene/scene_worker.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/material_sample_cache.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/shape/shape.hpp"
#include "base/spectrum/rgb.inl"
#include "base/math/math.hpp"
#include "base/math/distribution/distribution_2d.inl"

namespace scene { namespace material { namespace display {

Emissionmap::Emissionmap(Sample_cache& sample_cache, const Sampler_settings& sampler_settings,
						 bool two_sided) :
	Material(sample_cache, sampler_settings, two_sided),
	average_emission_(float3(-1.f)) {}

const material::Sample& Emissionmap::sample(float3_p wo, const Renderstate& rs,
										 const Worker& worker, Sampler_filter filter) {
	auto& sample = sample_cache_.get<Sample>(worker.id());

	sample.set_basis(rs.geo_n, wo);

	sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

	auto& sampler = worker.sampler_2D(sampler_key(), filter);

	float3 radiance = emission_map_.sample_3(sampler, rs.uv);
	sample.layer_.set(emission_factor_ * radiance, f0_, roughness_);

	return sample;
}

float3 Emissionmap::sample_radiance(float3_p /*wi*/, float2 uv, float /*area*/,
									float /*time*/, const Worker& worker,
									Sampler_filter filter) const {
	auto& sampler = worker.sampler_2D(sampler_key(), filter);

	return emission_factor_ * emission_map_.sample_3(sampler, uv);
}

float3 Emissionmap::average_radiance(float /*area*/) const {
	return average_emission_;
}

bool Emissionmap::has_emission_map() const {
	return emission_map_.is_valid();
}

float2 Emissionmap::radiance_sample(float2 r2, float& pdf) const {
	float2 uv = distribution_.sample_continuous(r2, pdf);

	pdf *= total_weight_;

	return uv;
}

float Emissionmap::emission_pdf(float2 uv, const Worker& worker,
								Sampler_filter filter) const {
	auto& sampler = worker.sampler_2D(sampler_key(), filter);

	return distribution_.pdf(sampler.address(uv)) * total_weight_ ;
}

void Emissionmap::prepare_sampling(const shape::Shape& shape, uint32_t /*part*/,
								   const Transformation& /*transformation*/,
								   float /*area*/, bool importance_sampling,
								   thread::Pool& /*pool*/) {
	if (average_emission_.x >= 0.f) {
		// Hacky way to check whether prepare_sampling has been called before
		// average_emission_ is initialized with negative values...
		return;
	}

	if (importance_sampling) {
		float3 average_radiance = float3(0.f);

		float total_weight = 0.f;

		auto d = emission_map_.texture()->dimensions_2();
		std::vector<float> luminance(d.x * d.y);

		float2 id(1.f / static_cast<float>(d.x), 1.f / static_cast<float>(d.y));

		auto texture = emission_map_.texture();

		for (int32_t y = 0, l = 0; y < d.y; ++y) {
			float v = id.y * (static_cast<float>(y) + 0.5f);

			for (int32_t x = 0; x < d.x; ++x, ++l) {
				float u = id.x * (static_cast<float>(x) + 0.5f);

				float uv_weight = shape.uv_weight(float2(u, v));

				float3 radiance = emission_factor_ * texture->at_3(x, y);

				average_radiance += uv_weight * radiance;

				total_weight += uv_weight;

				luminance[l] = uv_weight * spectrum::luminance(radiance);
			}
		}

		average_emission_ = average_radiance / total_weight;

		total_weight_ = total_weight;

		distribution_.init(luminance.data(), d);
	} else {
		average_emission_ = emission_factor_ * emission_map_.texture()->average_3();
	}

	if (is_two_sided()) {
		average_emission_ *= 2.f;
	}
}

size_t Emissionmap::num_bytes() const {
	return sizeof(*this);
}

void Emissionmap::set_emission_map(const Texture_adapter& emission_map) {
	emission_map_ = emission_map;
}

void Emissionmap::set_emission_factor(float emission_factor) {
	emission_factor_ = emission_factor;
}

void Emissionmap::set_roughness(float roughness) {
	roughness_ = roughness;
}

void Emissionmap::set_ior(float ior) {
	f0_ = fresnel::schlick_f0(1.f, ior);
}

}}}
