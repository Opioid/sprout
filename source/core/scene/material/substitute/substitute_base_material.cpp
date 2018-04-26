#include "substitute_base_material.hpp"
#include "substitute_sample.hpp"
#include "image/texture/texture_adapter.inl"
#include "scene/scene_worker.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material_sample.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "base/math/vector4.inl"

namespace scene::material::substitute {

Material_base::Material_base(const Sampler_settings& sampler_settings, bool two_sided) :
	material::Material(sampler_settings, two_sided) {}

float3 Material_base::sample_radiance(f_float3 /*wi*/, float2 uv, float /*area*/,
									  float /*time*/, Sampler_filter filter,
									  const Worker& worker) const {
	if (emission_map_.is_valid()) {
		// For some reason Clang needs this to find inherited Material::sampler_key_
		auto& sampler = worker.sampler_2D(sampler_key(), filter);
		return emission_factor_ * emission_map_.sample_3(sampler, uv);
	} else {
		return float3(0.f);
	}
}

float3 Material_base::average_radiance(float /*area*/) const {
	if (emission_map_.is_valid()) {
		return emission_factor_ * emission_map_.texture()->average_3();
	} else {
		return float3(0.f);
	}
}

bool Material_base::has_emission_map() const {
	return emission_map_.is_valid();
}

float Material_base::ior() const {
	return ior_;
}

void Material_base::set_color_map(const Texture_adapter& color_map) {
	color_map_ = color_map;
}

void Material_base::set_normal_map(const Texture_adapter& normal_map) {
	normal_map_ = normal_map;
}

void Material_base::set_surface_map(const Texture_adapter& surface_map) {
	surface_map_ = surface_map;
}

void Material_base::set_emission_map(const Texture_adapter& emission_map) {
	emission_map_ = emission_map;
}

void Material_base::set_color(const float3& color) {
	color_ = color;
}

void Material_base::set_ior(float ior, float external_ior) {
	ior_ = ior;
	constant_f0_ = fresnel::schlick_f0(external_ior, ior);
}

void Material_base::set_roughness(float roughness) {
	roughness_ = ggx::clamp_roughness(roughness);
}

void Material_base::set_metallic(float metallic) {
	metallic_ = metallic;
}

void Material_base::set_emission_factor(float emission_factor) {
	emission_factor_ = emission_factor;
}

}
