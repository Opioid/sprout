#include "display_material_animated.hpp"
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

Material_animated::Material_animated(Generic_sample_cache<Sample>& cache,
									 const Sampler_settings& sampler_settings,
									 bool two_sided,
									 const Adapter_2D& emission_map,
									 float animation_duration) :
	material::Typed_material<Generic_sample_cache<Sample>>(cache, sampler_settings, two_sided),
	emission_map_(emission_map),
	average_emissions_(emission_map.texture()->num_elements()),
	frame_length_(animation_duration / static_cast<float>(emission_map_.texture()->num_elements())),
	element_(0) {
	for (auto& ae : average_emissions_) {
		ae = float3(-1.f, -1.f, -1.f);
	}
}

void Material_animated::tick(float absolute_time, float /*time_slice*/) {
	element_ = static_cast<int32_t>(absolute_time / frame_length_) %
									emission_map_.texture()->num_elements();
}

const material::Sample& Material_animated::sample(float3_p wo, const Renderstate& rs,
												  const Worker& worker, Sampler_filter filter) {
	auto& sample = cache_.get(worker.id());

	sample.set_basis(rs.geo_n, wo);

	sample.layer_.set_basis(rs.t, rs.b, rs.n);

	if (emission_map_.is_valid()) {
		auto& sampler = worker.sampler(sampler_key_, filter);

		float3 radiance = emission_map_.sample_3(sampler, rs.uv, element_);
		sample.layer_.set(emission_factor_ * radiance, f0_, roughness_);
	} else {
		sample.layer_.set(emission_factor_ * emission_, f0_, roughness_);
	}

	return sample;
}

float3 Material_animated::sample_radiance(float3_p /*wi*/, float2 uv,
										  float /*area*/, float /*time*/,
										  const Worker& worker, Sampler_filter filter) const {
	auto& sampler = worker.sampler(sampler_key_, filter);
	return emission_factor_ * emission_map_.sample_3(sampler, uv, element_);
}

float3 Material_animated::average_radiance(float /*area*/) const {
	return average_emissions_[element_];
}

bool Material_animated::has_emission_map() const {
	return emission_map_.is_valid();
}

float2 Material_animated::radiance_sample(float2 r2, float& pdf) const {
	float2 uv = distribution_.sample_continuous(r2, pdf);

	pdf *= total_weight_;

	return uv;
}

float Material_animated::emission_pdf(float2 uv, const Worker& worker,
									  Sampler_filter filter) const {
	auto& sampler = worker.sampler(sampler_key_, filter);

	return distribution_.pdf(sampler.address(uv)) * total_weight_;
}

float Material_animated::opacity(float2 uv, float /*time*/,
								 const Worker& worker, Sampler_filter filter) const {
	if (mask_.is_valid()) {
		auto& sampler = worker.sampler(sampler_key_, filter);
		return mask_.sample_1(sampler, uv, element_);
	} else {
		return 1.f;
	}
}

void Material_animated::prepare_sampling(const shape::Shape& /*shape*/, uint32_t /*part*/,
										 const Transformation& /*transformation*/,
										 float /*area*/, thread::Pool& /*pool*/) {
	if (average_emissions_[element_].x >= 0.f) {
		// Hacky way to check whether prepare_sampling has been called before
		// average_emission_ is initialized with negative values...
		return;
	}

	/*	average_emission_ = float3::identity;

		auto d = emission_->dimensions();
		std::vector<float> luminance(d.x * d.y);

		total_weight_ = 0.f;

		for (int32_t y = 0, l = 0; y < d.y; ++y) {
			float sin_theta = std::sin(((static_cast<float>(y) + 0.5f) / static_cast<float>(d.y)) * math::Pi);

			for (int32_t x = 0; x < d.x; ++x, ++l) {
				float3 radiance = emission_factor_ * emission_->at_3(x, y);

				luminance[l] = spectrum::luminance(radiance);

				average_emission_ += sin_theta * radiance;

				total_weight_ += sin_theta;
			}
		}

		average_emission_ /= total_weight_;

		distribution_.init(luminance.data(), d);*/
}

void Material_animated::prepare_sampling() {
	if (average_emissions_[element_].x >= 0.f) {
		// Hacky way to check whether prepare_sampling has been called before
		// average_emission_ is initialized with negative values...
		return;
	}

	average_emissions_[element_] = emission_factor_ * emission_map_.texture()->average_3(element_);

	if (is_two_sided()) {
		average_emissions_[element_] *= 2.f;
	}
}

bool Material_animated::is_animated() const {
	return true;
}

size_t Material_animated::num_bytes() const {
	return sizeof(*this);
}

void Material_animated::set_emission_factor(float emission_factor) {
	emission_factor_ = emission_factor;
}

void Material_animated::set_roughness(float roughness) {
	roughness_ = roughness;
}

void Material_animated::set_ior(float ior) {
	f0_ = fresnel::schlick_f0(1.f, ior);
}

}}}
