#include "substitute_translucent_material.hpp"
#include "substitute_translucent_sample.hpp"
#include "substitute_base_material.inl"
#include "substitute_base_sample.inl"
#include "image/texture/texture_adapter.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.hpp"
#include "base/math/vector4.inl"

namespace scene { namespace material { namespace substitute {

Material_translucent::Material_translucent(Sample_cache& sample_cache,
										   const Sampler_settings& sampler_settings) :
	Material_base(sample_cache, sampler_settings, true) {}

const material::Sample& Material_translucent::sample(const float3& wo, const Renderstate& rs,
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
