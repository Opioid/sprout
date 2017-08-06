#include "metallic_paint_material.hpp"
#include "metallic_paint_sample.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "image/texture/texture_adapter.inl"
#include "scene/material/material_sample.inl"
#include "scene/material/coating/coating.inl"
#include "base/math/vector4.inl"

namespace scene { namespace material { namespace metallic_paint {

Material::Material(const Sampler_settings& sampler_settings, bool two_sided) :
	material::Material(sampler_settings, two_sided) {}

const material::Sample& Material::sample(const float3& wo, const Renderstate& rs,
										 Worker& worker, Sampler_filter filter) {
	auto& sample = worker.sample<Sample>();

	sample.set_basis(rs.geo_n, wo);
/*
	if (normal_map_) {
		auto& sampler = worker.sampler_2D(sampler_key_, filter);

		float3 nm = sampler.sample_3(*normal_map_, rs.uv);
		float3 n  = math::normalize(rs.tangent_to_world(nm));
		sample.layer_.set_tangent_frame(rs.t, rs.b, n);
	} else {*/
		sample.base_.set_tangent_frame(rs.t, rs.b, rs.n);

		sample.coating_.set_tangent_frame(rs.t, rs.b, rs.n);
//	}

	auto& sampler = worker.sampler_2D(sampler_key(), filter);

	if (flakes_normal_map_.is_valid()) {
		float3 nm = flakes_normal_map_.sample_3(sampler, rs.uv);
		float3 n = math::normalize(rs.tangent_to_world(nm));

		sample.flakes_.set_tangent_frame(n);
	} else {
		sample.flakes_.set_tangent_frame(rs.t, rs.b, rs.n);
	}

	sample.base_.set(color_a_, color_b_, alpha_, alpha2_);

	float flakes_weight;
	if (flakes_mask_.is_valid()) {
		flakes_weight = flakes_mask_.sample_1(sampler, rs.uv);
	} else {
		flakes_weight = 1.f;
	}

//	sample.flakes_.weight = 0.f;// - math::dot(sample.base_.n, sample.flakes_.n);

	sample.flakes_.set(flakes_ior_, flakes_absorption_,
					   flakes_alpha_, flakes_alpha2_, flakes_weight);

	sample.coating_.set_color_and_weight(coating_.color_, coating_.weight_);

	sample.coating_.set(coating_.f0_, coating_.alpha_, coating_.alpha2_);

	return sample;
}

size_t Material::num_bytes() const {
	return sizeof(*this);
}

void Material::set_color(const float3& a, const float3& b) {
	color_a_ = a;
	color_b_ = b;
}

void Material::set_roughness(float roughness) {
	roughness = ggx::clamp_roughness(roughness);
	const float alpha = roughness * roughness;
	alpha_  = alpha;
	alpha2_ = alpha * alpha;
}

void Material::set_flakes_mask(const Texture_adapter& mask) {
	flakes_mask_ = mask;
}

void Material::set_flakes_normal_map(const Texture_adapter& normal_map) {
	flakes_normal_map_ = normal_map;
}

void Material::set_flakes_ior(const float3& ior) {
	flakes_ior_ = ior;
}

void Material::set_flakes_absorption(const float3& absorption) {
	flakes_absorption_ = absorption;
}

void Material::set_flakes_roughness(float roughness) {
	const float alpha = roughness * roughness;
	flakes_alpha_  = alpha;
	flakes_alpha2_ = alpha * alpha;
}

void Material::set_coating_weight(float weight) {
	coating_.weight_ = weight;
}

void Material::set_coating_color(const float3& color) {
	coating_.color_ = color;
}

void Material::set_clearcoat(float ior, float roughness) {
	coating_.f0_ = fresnel::schlick_f0(1.f, ior);
	const float alpha = roughness * roughness;
	coating_.alpha_  = alpha;
	coating_.alpha2_ = alpha * alpha;
}

}}}
