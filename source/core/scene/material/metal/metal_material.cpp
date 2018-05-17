#include "metal_material.hpp"
#include "metal_sample.hpp"
#include "image/texture/texture_adapter.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "scene/material/material_helper.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/ggx/ggx.inl"
#include "base/math/vector4.inl"

namespace scene::material::metal {

Material_isotropic::Material_isotropic(Sampler_settings const& sampler_settings, bool two_sided) :
	Material(sampler_settings, two_sided) {}

const material::Sample& Material_isotropic::sample(f_float3 wo, Renderstate const& rs,
												   Sampler_filter filter,
												   sampler::Sampler& /*sampler*/,
												   Worker const& worker) const {
	auto& sample = worker.sample<Sample_isotropic>();

	sample.set_basis(rs.geo_n, wo);

	if (normal_map_.is_valid()) {
		auto& sampler = worker.sampler_2D(sampler_key(), filter);
		float3 const n = sample_normal(wo, rs, normal_map_, sampler);
		sample.layer_.set_tangent_frame(n);
	} else {
		sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);
	}

	sample.layer_.set(ior_, absorption_, roughness_);

	return sample;
}

float Material_isotropic::ior() const {
	return ior_[0];
}

size_t Material_isotropic::num_bytes() const {
	return sizeof(*this);
}

void Material_isotropic::set_normal_map(Texture_adapter const& normal_map) {
	normal_map_ = normal_map;
}

void Material_isotropic::set_ior(float3 const& ior) {
	ior_ = ior;
}

void Material_isotropic::set_absorption(float3 const& absorption) {
	absorption_ = absorption;
}

void Material_isotropic::set_roughness(float roughness) {
	roughness_ = ggx::clamp_roughness(roughness);
}

size_t Material_isotropic::sample_size() {
	return sizeof(Sample_isotropic);
}

Material_anisotropic::Material_anisotropic(Sampler_settings const& sampler_settings,
										   bool two_sided) :
	Material(sampler_settings, two_sided) {}

const material::Sample& Material_anisotropic::sample(f_float3 wo, Renderstate const& rs,
													 Sampler_filter filter,
													 sampler::Sampler& /*sampler*/,
													 Worker const& worker) const {
	auto& sample = worker.sample<Sample_anisotropic>();

	auto& sampler = worker.sampler_2D(sampler_key(), filter);

	sample.set_basis(rs.geo_n, wo);

	if (normal_map_.is_valid()) {
		float3 nm = normal_map_.sample_3(sampler, rs.uv);
		float3 n  = math::normalize(rs.tangent_to_world(nm));

		sample.layer_.set_tangent_frame(n);
	} else if (direction_map_.is_valid()) {
		float2 tm = direction_map_.sample_2(sampler, rs.uv);
		float3 t  = math::normalize(rs.tangent_to_world(tm));
		float3 b  = math::cross(rs.n, t);

		sample.layer_.set_tangent_frame(t, b, rs.n);
	} else {
		sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);
	}

	sample.layer_.set(ior_, absorption_, roughness_);

	return sample;
}

float Material_anisotropic::ior() const {
	return ior_[0];
}

size_t Material_anisotropic::num_bytes() const {
	return sizeof(*this);
}

void Material_anisotropic::set_normal_map(Texture_adapter const& normal_map) {
	normal_map_ = normal_map;
}

void Material_anisotropic::set_direction_map(Texture_adapter const& direction_map) {
	direction_map_ = direction_map;
}

void Material_anisotropic::set_ior(float3 const& ior) {
	ior_ = ior;
}

void Material_anisotropic::set_absorption(float3 const& absorption) {
	absorption_ = absorption;
}

void Material_anisotropic::set_roughness(float2 roughness) {
	roughness_ = float2(ggx::clamp_roughness(roughness[0]),
						ggx::clamp_roughness(roughness[1]));
}

size_t Material_anisotropic::sample_size() {
	return sizeof(Sample_anisotropic);
}

}
