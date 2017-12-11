#include "volumetric_material.hpp"
#include "volumetric_sample.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "scene/material/material_sample.inl"

namespace scene::material::volumetric {

Material::Material(const Sampler_settings& sampler_settings) :
	material::Material(sampler_settings, true) {}

Material::~Material() {}


const material::Sample& Material::sample(const float3& wo, const Renderstate& rs,
										 Sampler_filter /*filter*/, const Worker& worker) {
	auto& sample = worker.sample<Sample>();

	sample.set_basis(rs.geo_n, wo);

	sample.layer_.set(anisotropy_);

	return sample;
}

float Material::phase(const float3& w, const float3& wp) const {
	const float g = anisotropy_;
	const float k = 1.55f * g - (0.55f * g) * (g * g);
	return phase_schlick(w, wp, k);
}

bool Material::is_volumetric() const {
	return true;
}

void Material::set_attenuation(const float3& absorption_color, const float3& scattering_color,
							   float distance) {
	absorption_coefficient_ = absorption_coefficient(absorption_color, distance);
	scattering_coefficient_ = scattering_coefficient(scattering_color, distance);
}

void Material::set_anisotropy(float anisotropy) {
	anisotropy_ = anisotropy;
}

float Material::phase_schlick(const float3& w, const float3& wp, float k) {
	const float d = 1.f - (k * math::dot(w, wp));
	return 1.f / (4.f * math::Pi) * (1.f - k * k) / (d * d);
}

}
