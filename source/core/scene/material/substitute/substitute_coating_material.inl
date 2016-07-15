#pragma once

#include "substitute_coating_material.hpp"
#include "substitute_base_material.inl"
#include "image/texture/texture_2d_adapter.inl"
#include "scene/scene_renderstate.hpp"

namespace scene { namespace material { namespace substitute {

template<typename Coating, typename Sample>
Material_coating<Coating, Sample>::Material_coating(
		Generic_sample_cache<Sample>& cache,
		const Sampler_settings& sampler_settings, bool two_sided) :
	Material_base<Sample>(cache, sampler_settings, two_sided) {}


template<typename Coating, typename Sample>
void Material_coating<Coating, Sample>::set_coating_weight_map(const Adapter_2D& weight_map) {
	coating_weight_map_ = weight_map;
}

template<typename Coating, typename Sample>
void Material_coating<Coating, Sample>::set_coating_normal_map(const Adapter_2D& normal_map) {
	coating_normal_map_ = normal_map;
}

template<typename Coating, typename Sample>
void Material_coating<Coating, Sample>::set_coating_weight(float weight) {
	coating_.weight = weight;
}

template<typename Coating, typename Sample>
void Material_coating<Coating, Sample>::set_coating_color(float3_p color) {
	coating_.color = color;
}

template<typename Coating, typename Sample>
void Material_coating<Coating, Sample>::set_coating_basis(const Renderstate& rs,
														  const Texture_sampler_2D& sampler,
														  Sample& sample) {
	if (Material_base<Sample>::normal_map_ == coating_normal_map_) {
		sample.coating_.set_basis(rs.t, rs.b, sample.layer_.n);
	} else if (coating_normal_map_.is_valid()) {
		float3 nm = coating_normal_map_.sample_3(sampler, rs.uv);
		float3 n  = math::normalized(rs.tangent_to_world(nm));

		sample.coating_.set_basis(rs.t, rs.b, n);
	} else {
		sample.coating_.set_basis(rs.t, rs.b, rs.n);
	}

	if (coating_weight_map_.is_valid()) {
		float weight = coating_weight_map_.sample_1(sampler, rs.uv);
		sample.coating_.set_color_and_weight(coating_.color, weight);
	} else {
		sample.coating_.set_color_and_weight(coating_.color, coating_.weight);
	}
}

}}}
