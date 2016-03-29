#pragma once

#include "substitute_material_base.hpp"
#include "substitute_sample.hpp"
#include "image/texture/texture_2d.hpp"
#include "image/texture/sampler/sampler_2d.hpp"
#include "scene/scene_worker.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/material_sample_cache.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/shape/geometry/differential.inl"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace substitute {

template<typename Sample>
Material_base<Sample>::Material_base(Generic_sample_cache<Sample>& cache,
									 std::shared_ptr<image::texture::Texture_2D> mask,
									 const Sampler_settings& sampler_settings, bool two_sided) :
	material::Typed_material<Generic_sample_cache<Sample>>(cache, mask, sampler_settings, two_sided) {}

template<typename Sample>
math::float3 Material_base<Sample>::sample_emission(math::float2 uv, float /*time*/,
													const Worker& worker, Sampler_settings::Filter filter) const {
	if (emission_map_) {
		// For some reason Clang needs this to find inherited Material::sampler_key_
		auto& sampler = worker.sampler(this->sampler_key_, filter);
		return emission_factor_ * sampler.sample_3(*emission_map_, uv);
	} else {
		return math::float3_identity;
	}
}

template<typename Sample>
math::float3 Material_base<Sample>::average_emission() const {
	if (emission_map_) {
		return emission_factor_ * emission_map_->average_3();
	} else {
		return math::float3_identity;
	}
}

template<typename Sample>
bool Material_base<Sample>::has_emission_map() const {
	return nullptr != emission_map_;
}

template<typename Sample>
void Material_base<Sample>::set_color_map(std::shared_ptr<image::texture::Texture_2D> color_map) {
	color_map_ = color_map;
}

template<typename Sample>
void Material_base<Sample>::set_normal_map(std::shared_ptr<image::texture::Texture_2D> normal_map) {
	normal_map_ = normal_map;
}

template<typename Sample>
void Material_base<Sample>::set_surface_map(std::shared_ptr<image::texture::Texture_2D> surface_map) {
	surface_map_ = surface_map;
}

template<typename Sample>
void Material_base<Sample>::set_emission_map(std::shared_ptr<image::texture::Texture_2D> emission_map) {
	emission_map_ = emission_map;
}

template<typename Sample>
void Material_base<Sample>::set_color(const math::float3& color) {
	color_ = color;
}

template<typename Sample>
void Material_base<Sample>::set_ior(float ior) {
	constant_f0_ = fresnel::schlick_f0(1.f, ior);
}

template<typename Sample>
void Material_base<Sample>::set_roughness(float roughness) {
	roughness_ = roughness;
}

template<typename Sample>
void Material_base<Sample>::set_metallic(float metallic) {
	metallic_ = metallic;
}

template<typename Sample>
void Material_base<Sample>::set_emission_factor(float emission_factor) {
	emission_factor_ = emission_factor;
}

}}}
