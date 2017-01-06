#pragma once

#include "substitute_base_material.hpp"
#include "substitute_sample.hpp"
#include "image/texture/texture_adapter.inl"
#include "scene/scene_worker.hpp"
#include "scene/scene_renderstate.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material_sample.inl"
#include "scene/material/material_sample_cache.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "base/math/vector.inl"

#include "scene/material/material_test.hpp"
#include "base/debug/assert.hpp"

namespace scene { namespace material { namespace substitute {

template<typename Sample>
Material_base<Sample>::Material_base(BSSRDF_cache& bssrdf_cache,
									 const Sampler_settings& sampler_settings,
									 bool two_sided, Sample_cache<Sample>& cache) :
	Typed_material<Sample_cache<Sample>>(bssrdf_cache, sampler_settings, two_sided, cache) {}

template<typename Sample>
float3 Material_base<Sample>::sample_radiance(float3_p /*wi*/, float2 uv, float /*area*/,
											  float /*time*/, const Worker& worker,
											  Sampler_filter filter) const {
	if (emission_map_.is_valid()) {
		// For some reason Clang needs this to find inherited Material::sampler_key_
		auto& sampler = worker.sampler_2D(this->sampler_key(), filter);
		return emission_factor_ * emission_map_.sample_3(sampler, uv);
	} else {
		return float3(0.f);
	}
}

template<typename Sample>
float3 Material_base<Sample>::average_radiance(float /*area*/) const {
	if (emission_map_.is_valid()) {
		return emission_factor_ * emission_map_.texture()->average_3();
	} else {
		return float3(0.f);
	}
}

template<typename Sample>
bool Material_base<Sample>::has_emission_map() const {
	return emission_map_.is_valid();
}

template<typename Sample>
void Material_base<Sample>::set_color_map(const Texture_adapter& color_map) {
	color_map_ = color_map;
}

template<typename Sample>
void Material_base<Sample>::set_normal_map(const Texture_adapter& normal_map) {
	normal_map_ = normal_map;
}

template<typename Sample>
void Material_base<Sample>::set_surface_map(const Texture_adapter& surface_map) {
	surface_map_ = surface_map;
}

template<typename Sample>
void Material_base<Sample>::set_emission_map(const Texture_adapter& emission_map) {
	emission_map_ = emission_map;
}

template<typename Sample>
void Material_base<Sample>::set_color(float3_p color) {
	color_ = color;
}

template<typename Sample>
void Material_base<Sample>::set_ior(float ior, float externeal_ior) {
	ior_ = ior;
	constant_f0_ = fresnel::schlick_f0(externeal_ior, ior);
}

template<typename Sample>
void Material_base<Sample>::set_roughness(float roughness) {
	roughness_ = ggx::clamp_roughness(roughness);
}

template<typename Sample>
void Material_base<Sample>::set_metallic(float metallic) {
	metallic_ = metallic;
}

template<typename Sample>
void Material_base<Sample>::set_emission_factor(float emission_factor) {
	emission_factor_ = emission_factor;
}

template<typename Sample>
void Material_base<Sample>::set_sample(float3_p wo, const Renderstate& rs,
									   const Texture_sampler_2D& sampler, Sample& sample) {
	sample.set_basis(rs.geo_n, wo);

	if (normal_map_.is_valid()) {
		float3 nm = normal_map_.sample_3(sampler, rs.uv);
		float3 n = math::normalized(rs.tangent_to_world(nm));

		SOFT_ASSERT(testing::check_normal_map(n, nm, rs.uv));

		sample.layer_.set_tangent_frame(n);
	} else {
		sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);
	}

	float3 color;
	if (color_map_.is_valid()) {
		color = color_map_.sample_3(sampler, rs.uv);
	} else {
		color = color_;
	}

	float2 surface;
	if (surface_map_.is_valid()) {
		surface = surface_map_.sample_2(sampler, rs.uv);
		surface.x = ggx::map_roughness(surface.x);
	} else {
		surface.x = roughness_;
		surface.y = metallic_;
	}

	float3 radiance;
	if (emission_map_.is_valid()) {
		radiance = emission_factor_ * emission_map_.sample_3(sampler, rs.uv);
	} else {
		radiance = math::float3_identity;
	}

	sample.layer_.set(color, radiance, ior_, constant_f0_, surface.x, surface.y);
}

}}}
