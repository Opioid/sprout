#include "substitute_subsurface_material.hpp"
#include "substitute_subsurface_sample.hpp"
#include "substitute_base_sample.inl"
#include "substitute_base_material.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.hpp"
#include "base/math/vector4.inl"

namespace scene { namespace material { namespace substitute {

Material_subsurface::Material_subsurface(const Sampler_settings& sampler_settings) :
	Material_base(sampler_settings, false) {}

const material::Sample& Material_subsurface::sample(const float3& wo, const Renderstate& rs,
													Worker& worker, Sampler_filter filter) {
	auto& sample = worker.sample_cache().get<Sample_subsurface>();

	auto& sampler = worker.sampler_2D(sampler_key(), filter);

	set_sample(wo, rs, sampler, sample);

	sample.set(material::absorption_coefficient(absorption_color_, attenuation_distance_),
			   material::scattering_coefficient(scattering_color_, attenuation_distance_),
			   ior_, rs.ior);

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

}}}
