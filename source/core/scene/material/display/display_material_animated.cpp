#include "display_material_animated.hpp"
#include "display_sample.hpp"
#include "image/texture/sampler/sampler_2d.hpp"
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
									 std::shared_ptr<image::texture::Texture_2D> mask,
									 const Sampler_settings& sampler_settings,
									 bool two_sided,
									 std::shared_ptr<image::texture::Texture_2D> emission_map,
									 float animation_duration) :
	material::Typed_material<Generic_sample_cache<Sample>>(cache, mask,
														   sampler_settings, two_sided),
	emission_map_(emission_map),
	average_emissions_(emission_map->num_elements()),
	frame_length_(animation_duration / static_cast<float>(emission_map_->num_elements())),
	element_(0) {
	for (auto& ae : average_emissions_) {
		ae = math::float3(-1.f, -1.f, -1.f);
	}
}

void Material_animated::tick(float absolute_time, float /*time_slice*/) {
	element_ = static_cast<int32_t>(absolute_time / frame_length_) % emission_map_->num_elements();
}

const material::Sample& Material_animated::sample(const shape::Hitpoint& hp, math::pfloat3 wo,
												  float /*area*/, float /*time*/, float /*ior_i*/,
												  const Worker& worker, Sampler_filter filter) {
	auto& sample = cache_.get(worker.id());

	sample.set_basis(hp.t, hp.b, hp.n, hp.geo_n, wo, two_sided_);

	if (emission_map_) {
		auto& sampler = worker.sampler(sampler_key_, filter);

		math::float3 radiance = sampler.sample_3(*emission_map_, hp.uv, element_);
		sample.set(emission_factor_ * radiance, f0_, roughness_);
	} else {
		sample.set(emission_factor_ * emission_, f0_, roughness_);
	}

	return sample;
}

math::float3 Material_animated::sample_radiance(math::pfloat3 /*wi*/, math::float2 uv,
												float /*area*/, float /*time*/,
												const Worker& worker, Sampler_filter filter) const {
	auto& sampler = worker.sampler(sampler_key_, filter);
	return emission_factor_ * sampler.sample_3(*emission_map_, uv, element_);
}

math::float3 Material_animated::average_radiance(float /*area*/) const {
	return average_emissions_[element_];
}

bool Material_animated::has_emission_map() const {
	return nullptr != emission_map_;
}

math::float2 Material_animated::radiance_importance_sample(math::float2 r2, float& pdf) const {
	math::float2 uv = distribution_.sample_continuous(r2, pdf);

	if (uv.y == 0.f) {
		pdf = 0.f;
	} else {
		float sin_theta = std::sin(uv.y * math::Pi);

		pdf *= total_weight_ / sin_theta;
	}

	return uv;
}

float Material_animated::emission_pdf(math::float2 uv, const Worker& worker,
									  Sampler_filter filter) const {
	if (uv.y == 0.f) {
		return 0.f;
	}

	auto& sampler = worker.sampler(sampler_key_, filter);

	float sin_theta = std::sin(uv.y * math::Pi);

	return distribution_.pdf(sampler.address(uv)) * (total_weight_ / sin_theta);
}

float Material_animated::opacity(math::float2 uv, float /*time*/,
								 const Worker& worker, Sampler_filter filter) const {
	if (mask_) {
		auto& sampler = worker.sampler(sampler_key_, filter);
		return sampler.sample_1(*mask_, uv, element_);
	} else {
		return 1.f;
	}
}

void Material_animated::prepare_sampling(bool spherical) {
	if (average_emissions_[element_].x >= 0.f) {
		// Hacky way to check whether prepare_sampling has been called before
		// average_emission_ is initialized with negative values...
		return;
	}

	if (spherical) {
	/*	average_emission_ = math::float3::identity;

		auto d = emission_->dimensions();
		std::vector<float> luminance(d.x * d.y);

		total_weight_ = 0.f;

		for (int32_t y = 0, l = 0; y < d.y; ++y) {
			float sin_theta = std::sin(((static_cast<float>(y) + 0.5f) / static_cast<float>(d.y)) * math::Pi);

			for (int32_t x = 0; x < d.x; ++x, ++l) {
				math::float3 radiance = emission_factor_ * emission_->at_3(x, y);

				luminance[l] = spectrum::luminance(radiance);

				average_emission_ += sin_theta * radiance;

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

bool Material_animated::is_animated() const {
	return true;
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
