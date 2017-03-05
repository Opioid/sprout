#pragma once

#include "substitute_base_material.hpp"
#include "substitute_sample.hpp"
#include "image/texture/texture_adapter.inl"
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
void Material_base::set_sample(float3_p wo, const Renderstate& rs,
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
		surface[0] = ggx::map_roughness(surface[0]);
	} else {
		surface[0] = roughness_;
		surface[1] = metallic_;
	}

	float3 radiance;
	if (emission_map_.is_valid()) {
		radiance = emission_factor_ * emission_map_.sample_3(sampler, rs.uv);
	} else {
		radiance = math::float3_identity;
	}

	sample.layer_.set(color, radiance, ior_, constant_f0_, surface[0], surface[1]);
}

}}}
