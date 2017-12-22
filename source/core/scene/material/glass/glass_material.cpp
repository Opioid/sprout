#include "glass_material.hpp"
#include "glass_sample.hpp"
#include "image/texture/texture_adapter.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "scene/material/material_sample.inl"
#include "base/math/vector4.inl"

namespace scene::material::glass {

Glass::Glass(const Sampler_settings& sampler_settings) :
	Material(sampler_settings, false) {}

const material::Sample& Glass::sample(const float3& wo, const Renderstate& rs,
									  Sampler_filter filter, const Worker& worker) const {
	auto& sample = worker.sample<Sample>();

	sample.set_basis(rs.geo_n, wo);

	if (normal_map_.is_valid()) {
		auto& sampler = worker.sampler_2D(sampler_key(), filter);

		float3 nm = normal_map_.sample_3(sampler, rs.uv);
		float3 n  = math::normalize(rs.tangent_to_world(nm));

		sample.layer_.set_tangent_frame(n);
	} else {
		sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);
	}

	sample.layer_.set(refraction_color_, absorption_color_, attenuation_distance_, ior_, rs.ior);

	return sample;
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

void Glass::set_absorption_color(const float3& color) {
	absorption_color_ = color;
}

void Glass::set_attenuation_distance(float attenuation_distance) {
	attenuation_distance_ = attenuation_distance;
}

void Glass::set_ior(float ior) {
	ior_ = ior;
}

}
