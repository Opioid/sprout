#include "volumetric_material.hpp"
#include "volumetric_sample.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "scene/material/material_attenuation.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/null/null_sample.hpp"

namespace scene::material::volumetric {

Material::Material(const Sampler_settings& sampler_settings) :
	material::Material(sampler_settings, true) {}

Material::~Material() {}

const material::Sample& Material::sample(f_float3 wo, const Renderstate& rs,
										 Sampler_filter /*filter*/, sampler::Sampler& /*sampler*/,
										 const Worker& worker) const {
	if (rs.subsurface) {
		auto& sample = worker.sample<Sample>();

		sample.set_basis(rs.geo_n, wo);

		sample.set(anisotropy_);

		return sample;
	}

	auto& sample = worker.sample<null::Sample>();

	sample.set_basis(rs.geo_n, wo);

	return sample;
}

float Material::ior() const {
	return 1.f;
}

void Material::set_attenuation(float3 const& absorption_color, float3 const& scattering_color,
							   float distance) {
	attenuation(absorption_color, scattering_color, distance,
				absorption_coefficient_, scattering_coefficient_);
}

void Material::set_anisotropy(float anisotropy) {
	anisotropy_ = std::clamp(anisotropy, -0.999f, 0.999f);
}

size_t Material::sample_size() {
	return sizeof(Sample);
}

}
