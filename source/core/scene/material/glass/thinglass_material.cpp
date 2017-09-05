#include "thinglass_material.hpp"
#include "thinglass_sample.hpp"
#include "image/texture/texture_adapter.inl"
#include "rendering/integrator/surface/integrator_helper.hpp"
#include "scene/material/material_helper.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "scene/material/material_sample.inl"
#include "base/math/vector4.inl"

namespace scene { namespace material { namespace glass {

Thinglass::Thinglass(const Sampler_settings& sampler_settings) :
	Material(sampler_settings, true) {}

const material::Sample& Thinglass::sample(const float3& wo, const Renderstate& rs,
										  Sampler_filter filter, Worker& worker) {
	auto& sample = worker.sample<Sample_thin>();

	sample.set_basis(rs.geo_n, wo);

	if (normal_map_.is_valid()) {
		auto& sampler = worker.sampler_2D(sampler_key(), filter);
		const float3 n = sample_normal(normal_map_, sampler, rs);
		sample.layer_.set_tangent_frame(n);
	} else {
		sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);
	}

	sample.layer_.set(refraction_color_, absorption_color_,
					  attenuation_distance_, ior_, rs.ior, thickness_);

	return sample;
}

float3 Thinglass::thin_absorption(const float3& wo, const float3& n, float2 uv, float time,
								  Sampler_filter filter, Worker& worker) const {
	const float3 a = material::absorption_coefficient(absorption_color_, attenuation_distance_);

	const float  n_dot_wi = clamp_abs_dot(wo, n);
	const float  approximated_distance = thickness_ / n_dot_wi;
	const float3 attenuation = rendering::attenuation(approximated_distance, a);

	return opacity(uv, time, filter, worker) * (1.f - refraction_color_ * attenuation);
}

bool Thinglass::has_tinted_shadow() const {
	return true;
}

size_t Thinglass::num_bytes() const {
	return sizeof(*this);
}

void Thinglass::set_normal_map(const Texture_adapter& normal_map) {
	normal_map_ = normal_map;
}

void Thinglass::set_refraction_color(const float3& color) {
	refraction_color_ = color;
}

void Thinglass::set_absorption_color(const float3& color) {
	absorption_color_ = color;
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
