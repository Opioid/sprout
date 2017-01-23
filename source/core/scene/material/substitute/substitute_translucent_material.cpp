#include "substitute_translucent_material.hpp"
#include "substitute_translucent_sample.hpp"
#include "substitute_base_material.inl"
#include "substitute_base_sample.inl"
#include "image/texture/texture_adapter.inl"
#include "rendering/integrator/surface/integrator_helper.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.hpp"

namespace scene { namespace material { namespace substitute {

Material_translucent::Material_translucent(Sample_cache& sample_cache,
										   const Sampler_settings& sampler_settings) :
	Material_base(sample_cache, sampler_settings, true) {}

const material::Sample& Material_translucent::sample(float3_p wo, const Renderstate& rs,
													 const Worker& worker,
													 Sampler_filter filter) {
	auto& sample = sample_cache_.get<Sample_translucent>(worker.id());

	auto& sampler = worker.sampler_2D(sampler_key(), filter);

	set_sample(wo, rs, sampler, sample);

	float thickness;

	thickness = thickness_;
	sample.set(sample.layer_.diffuse_color_, thickness, attenuation_distance_);

	return sample;
}

float3 Material_translucent::thin_absorption(float3_p wo, float3_p n, float2 uv, float time,
											 const Worker& worker, Sampler_filter filter) const {
	float3 color;
	if (color_map_.is_valid()) {
		auto& sampler = worker.sampler_2D(sampler_key(), filter);
		color = color_map_.sample_3(sampler, uv);
	} else {
		color = color_;
	}

	float3 a = material::Sample::attenuation(color, attenuation_distance_);

	float n_dot_wi = material::Sample::absolute_clamped_dot(wo, n);
	float approximated_distance = thickness_ / n_dot_wi;
	float3 attenuation = rendering::attenuation(approximated_distance, a);

	return opacity(uv, time, worker, filter) * (1.f - /*refraction_color_ **/ attenuation);
}

bool Material_translucent::is_translucent() const {
	return true;
}

size_t Material_translucent::num_bytes() const {
	return sizeof(*this);
}

void Material_translucent::set_thickness(float thickness) {
	thickness_ = thickness;
}

void Material_translucent::set_attenuation_distance(float attenuation_distance) {
	attenuation_distance_ = attenuation_distance;
}

}}}
