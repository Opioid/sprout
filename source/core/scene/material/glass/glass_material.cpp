#include "glass_material.hpp"
#include "glass_sample.hpp"
#include "image/texture/texture_adapter.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "scene/material/material_attenuation.hpp"
#include "scene/material/material_helper.hpp"
#include "scene/material/material_sample.inl"
#include "base/math/vector4.inl"

namespace scene::material::glass {

Glass::Glass(const Sampler_settings& sampler_settings) :
	Material(sampler_settings, false) {}

const material::Sample& Glass::sample(const float3& wo, const Renderstate& rs,
									  Sampler_filter filter, sampler::Sampler& /*sampler*/,
									  const Worker& worker) const {
	auto& sample = worker.sample<Sample>();

	sample.set_basis(rs.geo_n, wo);

	if (normal_map_.is_valid()) {
		auto& sampler = worker.sampler_2D(sampler_key(), filter);
		const float3 n = sample_normal(wo, rs, normal_map_, sampler);
		sample.layer_.set_tangent_frame(n);
	} else {
		sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);
	}

	sample.layer_.set(refraction_color_, absorption_color_, attenuation_distance_, ior_, rs.ior);

	return sample;
}

float3 Glass::absorption_coefficient(float2 /*uv*/, Sampler_filter /*filter*/, const Worker& /*worker*/) const {
	return absorption_coefficient_;
}

float Glass::ior() const {
	return ior_;
}

bool Glass::is_scattering_volume() const {
	return false;
}

size_t Glass::num_bytes() const {
	return sizeof(*this);
}

void Glass::set_normal_map(const Texture_adapter& normal_map) {
	normal_map_ = normal_map;
}

void Glass::set_refraction_color(const float3& color) {
	refraction_color_ = color;
}

void Glass::set_attenuation(const float3& absorption_color, float distance) {
	absorption_color_ = absorption_color;

	absorption_coefficient_ = extinction_coefficient(absorption_color, distance);

	attenuation_distance_ = distance;
}

void Glass::set_ior(float ior) {
	ior_ = ior;
}

size_t Glass::sample_size() {
	return sizeof(Sample);
}

}
