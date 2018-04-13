#include "material.hpp"
#include "bssrdf.hpp"
#include "image/texture/texture_adapter.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.hpp"
#include "base/json/json.hpp"
#include "base/math/vector4.inl"
#include "base/spectrum/discrete.inl"
#include "base/spectrum/xyz.hpp"

namespace scene::material {

Material::Material(const Sampler_settings& sampler_settings, bool two_sided) :
	sampler_key_(sampler_settings.key()),
	two_sided_(two_sided) {}

Material::~Material() {}

void Material::set_mask(const Texture_adapter& mask) {
	mask_ = mask;
}

void Material::set_parameters(const json::Value& parameters) {
	for (auto& n : parameters.GetObject()) {
		set_parameter(n.name.GetString(), n.value);
	}
}

void Material::compile() {}

void Material::tick(float /*absolute_time*/, float /*time_slice*/) {}

float3 Material::sample_radiance(const float3& /*wi*/, float2 /*uv*/, float /*area*/,
								 float /*time*/, Sampler_filter /*filter*/,
								 const Worker& /*worker*/) const {
	return float3(0.f);
}

float3 Material::average_radiance(float /*area*/) const {
	return float3(0.f);
}

bool Material::has_emission_map() const {
	return false;
}

Material::Sample_2D Material::radiance_sample(float2 r2) const {
	return { r2, 1.f };
}

float Material::emission_pdf(float2 /*uv*/, Sampler_filter /*filter*/,
							 const Worker& /*worker*/) const {
	return 1.f;
}

float Material::opacity(float2 uv, float /*time*/, Sampler_filter filter,
						const Worker& worker) const {
	if (mask_.is_valid()) {
		auto& sampler = worker.sampler_2D(sampler_key_, filter);
		return mask_.sample_1(sampler, uv);
	} else {
		return 1.f;
	}
}

float3 Material::thin_absorption(const float3& /*wo*/, const float3& /*n*/, float2 uv, float time,
								 Sampler_filter filter, const Worker& worker) const {
	return float3(opacity(uv, time, filter, worker));
}

float3 Material::emission(const Transformation& /*transformation*/, const math::Ray& /*ray*/,
						  float /*step_size*/, rnd::Generator& /*rng*/,
						  Sampler_filter /*filter*/, const Worker& /*worker*/) const {
	return float3::identity();
}

float3 Material::optical_depth(const Transformation& /*transformation*/, const math::AABB& /*aabb*/,
							   const math::Ray& /*ray*/, float /*step_size*/,
							   rnd::Generator& /*rng*/, Sampler_filter /*filter*/,
							   const Worker& /*worker*/) const {
	return float3::identity();
}

float3 Material::absorption(float2 /*uv*/, Sampler_filter /*filter*/,
							const Worker& /*worker*/) const {
	return float3::identity();
}

void Material::extinction(float2 /*uv*/, Sampler_filter /*filter*/, const Worker& /*worker*/,
						  float3& absorption, float3& scattering) const {
	absorption = float3::identity();
	scattering = float3::identity();
}

void Material::extinction(const Transformation& /*transformation*/, const float3& /*p*/,
						  Sampler_filter /*filter*/, const Worker& /*worker*/,
						  float3& absorption, float3& scattering) const {
	absorption = float3::identity();
	scattering = float3::identity();
}

float3 Material::max_extinction() const {
	return float3::identity();
}

bool Material::is_heterogeneous_volume() const {
	return false;
}

bool Material::is_scattering_volume() const {
	return true;
}

void Material::prepare_sampling(const shape::Shape& /*shape*/, uint32_t /*part*/,
								const Transformation& /*transformation*/, float /*area*/,
								bool /*importance_sampling*/, thread::Pool& /*pool*/) {}

bool Material::is_animated() const {
	return false;
}

bool Material::has_tinted_shadow() const {
	return false;
}

bool Material::is_volumetric() const {
	return false;
}

uint32_t Material::sampler_key() const {
	return sampler_key_;
}

bool Material::is_masked() const {
	return mask_.is_valid();
}

bool Material::is_emissive() const {
	if (has_emission_map()) {
		return true;
	}

	const float3 e = average_radiance(1.f);
	return math::any_greater_zero(e);
}

bool Material::is_two_sided() const {
	return two_sided_;
}

void Material::set_parameter(const std::string& /*name*/, const json::Value& /*value*/) {}

float3 Material::rainbow_[Num_bands];

void Material::init_rainbow() {
	Spectrum::init(380.f, 720.f);

//	const float start = Spectrum::start_wavelength();
//	const float end   = Spectrum::end_wavelength();

	float3 sum_rgb(0.f);
	for (uint32_t i = 0; i < Num_bands; ++i) {
	//	const float wl = start + (end - start) * ((static_cast<float>(i) + 0.5f) / nb);
		Spectrum temp;
		temp.clear(0.f);
	//	temp.set_at_wavelength(wl, 1.f);
		temp.set_bin(i, 1.f);
		const float3 rgb = spectrum::XYZ_to_linear_RGB(temp.normalized_XYZ());
		rainbow_[i] = math::saturate(rgb);
		sum_rgb += rgb;
	}

	constexpr float nb = static_cast<float>(Num_bands);

	// now we hack-normalize it
	for (uint32_t i = 0; i < Num_bands; ++i) {
		rainbow_[i][0] *= nb / sum_rgb[0];
		rainbow_[i][1] *= nb / sum_rgb[1];
		rainbow_[i][2] *= nb / sum_rgb[2];
	}
}

float3 Material::spectrum_at_wavelength(float lambda, float value) {
	const float start = Spectrum::start_wavelength();
	const float end   = Spectrum::end_wavelength();
	const float nb    = static_cast<float>(Num_bands);

	const uint32_t idx = static_cast<uint32_t>(((lambda - start) / (end - start)) * nb);
	if (idx >= Num_bands) {
		return float3::identity();
	} else {
		const float weight = value * (1.f / 3.f);
		return weight * rainbow_[idx];
	}
}

}
