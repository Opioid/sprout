#include "glass_thin_material.hpp"
#include "glass_thin_sample.hpp"
#include "image/texture/texture_adapter.inl"
#include "rendering/integrator/integrator_helper.hpp"
#include "scene/material/material_attenuation.hpp"
#include "scene/material/material_helper.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "scene/material/material_sample.inl"
#include "base/math/vector4.inl"

namespace scene::material::glass {

Glass_thin::Glass_thin(const Sampler_settings& sampler_settings) :
	Material(sampler_settings, true) {}

const material::Sample& Glass_thin::sample(const float3& wo, const Renderstate& rs,
										  Sampler_filter filter, sampler::Sampler& /*sampler*/,
										  const Worker& worker) const {
	auto& sample = worker.sample<Sample_thin>();

	sample.set_basis(rs.geo_n, wo);

	if (normal_map_.is_valid()) {
		auto& sampler = worker.sampler_2D(sampler_key(), filter);
		const float3 n = sample_normal(wo, rs, normal_map_, sampler);
		sample.layer_.set_tangent_frame(n);
	} else {
		sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);
	}

	sample.layer_.set(refraction_color_, absorption_coefficient_,
					  ior_, rs.ior, thickness_);

	return sample;
}

float3 Glass_thin::thin_absorption(const float3& wo, const float3& n, float2 uv, float time,
								  Sampler_filter filter, const Worker& worker) const {
	const float  n_dot_wi = clamp_abs_dot(wo, n);
	const float  approximated_distance = thickness_ / n_dot_wi;
	const float3 attenuation = rendering::attenuation(approximated_distance,
													  absorption_coefficient_);

	return opacity(uv, time, filter, worker) * (1.f - refraction_color_ * attenuation);
}

float Glass_thin::ior() const {
	return ior_;
}

bool Glass_thin::has_tinted_shadow() const {
	return true;
}

bool Glass_thin::is_scattering_volume() const {
	return false;
}

size_t Glass_thin::num_bytes() const {
	return sizeof(*this);
}

void Glass_thin::set_normal_map(const Texture_adapter& normal_map) {
	normal_map_ = normal_map;
}

void Glass_thin::set_refraction_color(const float3& color) {
	refraction_color_ = color;
}

void Glass_thin::set_attenuation(const float3& absorption_color, float distance) {
	absorption_coefficient_ = extinction_coefficient(absorption_color, distance);
}
void Glass_thin::set_ior(float ior) {
	ior_ = ior;
}

void Glass_thin::set_thickness(float thickness) {
	thickness_ = thickness;
}

size_t Glass_thin::sample_size() {
	return sizeof(Sample_thin);
}

}
