#include "substitute_translucent_material.hpp"
#include "substitute_translucent_sample.hpp"
#include "substitute_base_material.inl"
#include "image/texture/texture_adapter.inl"
#include "scene/scene_renderstate.hpp"

namespace scene { namespace material { namespace substitute {

Material_translucent::Material_translucent(Sample_cache<Sample_translucent>& cache,
										   const Sampler_settings& sampler_settings,
										   bool two_sided) :
	Material_base<Sample_translucent>(cache, sampler_settings, two_sided) {}

const material::Sample& Material_translucent::sample(float3_p wo, const Renderstate& rs,
													 const Worker& worker,
													 Sampler_filter filter) {
	auto& sample = cache_.get(worker.id());

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
