#include "cloth_material.hpp"
#include "cloth_sample.hpp"
#include "image/texture/texture_adapter.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "scene/material/material_sample.inl"
#include "base/math/vector4.inl"

namespace scene::material::cloth {

Material::Material(const Sampler_settings& sampler_settings, bool two_sided) :
	material::Material(sampler_settings, two_sided) {}

const material::Sample& Material::sample(f_float3 wo, const Renderstate& rs,
										 Sampler_filter filter, sampler::Sampler& /*sampler*/,
										 const Worker& worker) const {
	auto& sample = worker.sample<Sample>();

	auto& sampler = worker.sampler_2D(sampler_key(), filter);

	sample.set_basis(rs.geo_n, wo);

	if (normal_map_.is_valid()) {
		float3 nm = normal_map_.sample_3(sampler, rs.uv);
		float3 n = math::normalize(rs.tangent_to_world(nm));
		sample.layer_.set_tangent_frame(rs.t, rs.b, n);
	} else {
		sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);
	}

	float3 color;
	if (color_map_.is_valid()) {
		color = color_map_.sample_3(sampler, rs.uv);
	} else {
		color = color_;
	}

	sample.layer_.set(color);

	return sample;
}

float Material::ior() const {
	return 1.5f;
}

size_t Material::num_bytes() const {
	return sizeof(*this);
}

void Material::set_color_map(const Texture_adapter& color_map) {
	color_map_ = color_map;
}

void Material::set_normal_map(const Texture_adapter& normal_map) {
	normal_map_ = normal_map;
}

void Material::set_color(const float3& color) {
	color_ = color;
}

size_t Material::sample_size() {
	return sizeof(Sample);
}

}
