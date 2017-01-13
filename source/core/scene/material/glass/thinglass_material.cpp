#include "thinglass_material.hpp"
#include "thinglass_sample.hpp"
#include "image/texture/texture_adapter.inl"
#include "scene/scene_renderstate.inl"
#include "scene/scene_worker.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/material_sample_cache.inl"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace glass {

Thinglass::Thinglass(Sample_cache& sample_cache, const Sampler_settings& sampler_settings) :
	Material(sample_cache, sampler_settings, false) {}

const material::Sample& Thinglass::sample(float3_p wo, const Renderstate& rs,
										  const Worker& worker, Sampler_filter filter) {
	auto& sample = sample_cache_.get<Sample_thin>(worker.id());

	sample.set_basis(rs.geo_n, wo);

	if (normal_map_.is_valid()) {
		auto& sampler = worker.sampler_2D(sampler_key(), filter);

		float3 nm = normal_map_.sample_3(sampler, rs.uv);
		float3 n  = math::normalized(rs.tangent_to_world(nm));

		sample.layer_.set_tangent_frame(n);
	} else {
		sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);
	}

	sample.layer_.set(refraction_color_, absorbtion_color_,
					  attenuation_distance_, ior_, rs.ior, thickness_);

	return sample;
}

size_t Thinglass::num_bytes() const {
	return sizeof(*this);
}

void Thinglass::set_normal_map(const Texture_adapter& normal_map) {
	normal_map_ = normal_map;
}

void Thinglass::set_refraction_color(float3_p color) {
	refraction_color_ = color;
}

void Thinglass::set_absorbtion_color(float3_p color) {
	absorbtion_color_ = color;
}

void Thinglass::set_attenuation_distance(float attenuation_distance) {
	attenuation_distance_ = attenuation_distance;
}

void Thinglass::set_ior(float ior) {
	ior_ = ior;
}

void Thinglass::set_thickness(float thickness) {
	thickness_ = thickness;
}

}}}
