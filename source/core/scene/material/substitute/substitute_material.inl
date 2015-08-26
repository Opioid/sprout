#pragma once

#include "substitute_material.hpp"

namespace scene { namespace material { namespace substitute {

void Base::set_color(const math::float3& color) {
	color_ = color;
}

void Base::set_color(std::shared_ptr<image::texture::Texture_2D> color) {
	color_map_ = color;
}

void Base::set_normal(std::shared_ptr<image::texture::Texture_2D> normal) {
	normal_map_ = normal;
}

void Base::set_roughness(float roughness) {
	roughness_ = roughness;
}

void Base::set_metallic(float metallic) {
	metallic_ = metallic;
}

void Base::set_surface(std::shared_ptr<image::texture::Texture_2D> surface) {
	surface_map_ = surface;
}

void Base::set_emission(std::shared_ptr<image::texture::Texture_2D> emission) {

}

template<bool Two_sided, bool Color_map, bool Normal_map, bool Surface_map, bool Emission_map>
Substitute<Two_sided, Color_map, Normal_map, Surface_map, Emission_map>::Substitute(
		Sample_cache& cache, std::shared_ptr<image::texture::Texture_2D> mask) : Material(cache, mask) {}

template<bool Two_sided, bool Color_map, bool Normal_map, bool Surface_map, bool Emission_map>
const material::Sample& Substitute<Two_sided, Color_map, Normal_map, Surface_map, Emission_map>::sample(
		const shape::Differential& dg, const math::float3& wo,
		const image::texture::sampler::Sampler_2D& sampler,
		uint32_t worker_id) {
	auto& sample = cache_.get<false>(worker_id);

	if (Normal_map) {
		math::float3 nm = sampler.sample_3(*normal_map_, dg.uv);
		math::float3 n = math::normalized(dg.tangent_to_world(nm));

		sample.template set_basis<Two_sided>(dg.t, dg.b, n, dg.geo_n, wo);
	} else {
		sample.template set_basis<Two_sided>(dg.t, dg.b, dg.n, dg.geo_n, wo);
	}

	math::float3 color;
	math::float2 surface;

	if (Color_map) {
		color = sampler.sample_3(*color_map_, dg.uv);
	} else {
		color = color_;
	}

	if (Surface_map) {
		surface  = sampler.sample_2(*surface_map_, dg.uv);
	} else {
		surface.x = roughness_;
		surface.y = metallic_;
	}

	sample.set(color, surface.x, surface.y);

	return sample;
}

template<bool Two_sided, bool Color_map, bool Normal_map, bool Surface_map, bool Emission_map>
math::float3 Substitute<Two_sided, Color_map, Normal_map, Surface_map, Emission_map>::sample_emission(
		math::float2 /*uv*/, const image::texture::sampler::Sampler_2D& /*sampler*/) const {
	return math::float3::identity;
}

template<bool Two_sided, bool Color_map, bool Normal_map, bool Surface_map, bool Emission_map>
math::float3 Substitute<Two_sided, Color_map, Normal_map, Surface_map, Emission_map>::average_emission() const {
	return math::float3::identity;
}

template<bool Two_sided, bool Color_map, bool Normal_map, bool Surface_map, bool Emission_map>
const image::texture::Texture_2D* Substitute<Two_sided, Color_map, Normal_map, Surface_map, Emission_map>::emission_map(
		) const {
	return nullptr;
}

}}}
