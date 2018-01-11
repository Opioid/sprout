#include "glass_rough_material.hpp"
#include "glass_rough_sample.hpp"
#include "image/texture/texture_adapter.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "scene/material/material_helper.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/ggx/ggx.inl"
#include "base/math/vector4.inl"

namespace scene::material::glass {

Glass_rough::Glass_rough(const Sampler_settings& sampler_settings) :
	Material(sampler_settings, false) {}

const material::Sample& Glass_rough::sample(const float3& wo, const Renderstate& rs,
											Sampler_filter filter, sampler::Sampler& /*sampler*/,
											const Worker& worker) const {
	auto& sample = worker.sample<Sample_rough>();

	sample.set_basis(rs.geo_n, wo);

	auto& sampler = worker.sampler_2D(sampler_key(), filter);

	if (normal_map_.is_valid()) {
		const float3 n = sample_normal(wo, rs, normal_map_, sampler);
		sample.layer_.set_tangent_frame(n);
	} else {
		sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);
	}

	float alpha;
	if (roughness_map_.is_valid()) {
		const float roughness = ggx::map_roughness(roughness_map_.sample_1(sampler, rs.uv));
		alpha = roughness * roughness;
	} else {
		alpha = alpha_;
	}

	sample.layer_.set(refraction_color_, absorption_color_,
					  attenuation_distance_, ior_, rs.ior, alpha);

	return sample;
}

size_t Glass_rough::num_bytes() const {
	return sizeof(*this);
}

void Glass_rough::set_normal_map(const Texture_adapter& normal_map) {
	normal_map_ = normal_map;
}

void Glass_rough::set_roughness_map(const Texture_adapter& roughness_map) {
	roughness_map_ = roughness_map;
}

void Glass_rough::set_refraction_color(const float3& color) {
	refraction_color_ = color;
}

void Glass_rough::set_absorption_color(const float3& color) {
	absorption_color_ = color;
}

void Glass_rough::set_attenuation_distance(float attenuation_distance) {
	attenuation_distance_ = attenuation_distance;
}

void Glass_rough::set_ior(float ior) {
	ior_ = ior;
}

void Glass_rough::set_roughness(float roughness) {
	roughness = ggx::clamp_roughness(roughness);
	alpha_ = roughness * roughness;
}

}
