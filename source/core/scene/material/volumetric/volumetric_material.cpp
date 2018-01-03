#include "volumetric_material.hpp"
#include "volumetric_sample.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "scene/material/material_attenuation.hpp"
#include "scene/material/material_sample.inl"

namespace scene::material::volumetric {

Material::Material(const Sampler_settings& sampler_settings) :
	material::Material(sampler_settings, true) {}

Material::~Material() {}

const material::Sample& Material::sample(const float3& wo, const Renderstate& rs,
										 Sampler_filter /*filter*/, const Worker& worker) const {
	auto& sample = worker.sample<Sample>();

	sample.set_basis(rs.geo_n, wo);

	sample.layer_.bssrdf.set(absorption_coefficient_, scattering_coefficient_,
							 anisotropy_);

	return sample;
}

bool Material::is_volumetric() const {
	return true;
}

void Material::set_attenuation(const float3& absorption_color, const float3& scattering_color,
							   float distance) {
	attenuation(absorption_color, scattering_color, distance,
				absorption_coefficient_, scattering_coefficient_);
}

void Material::set_anisotropy(float anisotropy) {
	anisotropy_ = anisotropy;
}

}
