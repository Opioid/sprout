#include "substitute_subsurface_material.hpp"
#include "substitute_base_sample.inl"
#include "substitute_base_material.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "base/math/vector4.inl"

namespace scene::material::substitute {

Material_subsurface::Material_subsurface(const Sampler_settings& sampler_settings) :
	Material_base(sampler_settings, false) {}

const material::Sample& Material_subsurface::sample(const float3& wo, const Renderstate& rs,
													Sampler_filter filter, const Worker& worker) {
	auto& sample = worker.sample<Sample_subsurface>();

	auto& sampler = worker.sampler_2D(sampler_key(), filter);

	set_sample(wo, rs, sampler, sample);

	sample.set(material::absorption_coefficient(absorption_color_, attenuation_distance_),
			   material::scattering_coefficient(scattering_color_, attenuation_distance_),
			   ior_);

	return sample;
}

size_t Material_subsurface::num_bytes() const {
	return sizeof(*this);
}

void Material_subsurface::set_absorption_color(const float3& color) {
	absorption_color_ = color;
}

void Material_subsurface::set_scattering_color(const float3& color) {
	scattering_color_ = color;
}

void Material_subsurface::set_attenuation_distance(float attenuation_distance) {
	attenuation_distance_ = attenuation_distance;
}

void Material_subsurface::set_ior(float ior, float external_ior) {
	Material_base::set_ior(ior, external_ior);

	ior_.ior_i_ = ior;
	ior_.ior_o_ = external_ior;
	const float eta_i = external_ior / ior;
	const float eta_t = ior / external_ior;
	ior_.eta_i_ = eta_i;
	ior_.eta_t_ = eta_t;
	ior_.sqrt_eta_i = fresnel::schlick_sqrt_eta(eta_i);
	ior_.sqrt_eta_i = fresnel::schlick_sqrt_eta(eta_t);
}

}
