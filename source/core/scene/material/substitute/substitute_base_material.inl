#pragma once

#include "substitute_base_material.hpp"
#include "substitute_sample.hpp"
#include "image/texture/texture_adapter.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material_helper.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "base/math/vector3.inl"

namespace scene::material::substitute {

template<typename Sample>
void Material_base::set_sample(const float3& wo, const Renderstate& rs,
							   const Texture_sampler_2D& sampler, Sample& sample) const {
	sample.set_basis(rs.geo_n, wo);

	if (normal_map_.is_valid()) {
		const float3 n = sample_normal(wo, rs, normal_map_, sampler);
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
		surface[0] = ggx::map_roughness(surface[0]);
	} else {
		surface[0] = roughness_;
		surface[1] = metallic_;
	}

	float3 radiance;
	if (emission_map_.is_valid()) {
		radiance = emission_factor_ * emission_map_.sample_3(sampler, rs.uv);
	} else {
		radiance = float3::identity();
	}



	sample.layer_.set(color, radiance, ior_, constant_f0_, surface[0], surface[1]);
}

}
