#pragma once

#include "substitute_coating_material.hpp"
#include "substitute_base_material.inl"
#include "image/texture/texture_adapter.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/material/material_helper.hpp"

namespace scene::material::substitute {

template<typename Coating>
Material_coating<Coating>::Material_coating(const Sampler_settings& sampler_settings,
											bool two_sided) :
	Material_base(sampler_settings, two_sided) {}

template<typename Coating>
size_t Material_coating<Coating>::num_bytes() const {
	return sizeof(*this);
}

template<typename Coating>
void Material_coating<Coating>::set_coating_weight_map(const Texture_adapter& weight_map) {
	coating_weight_map_ = weight_map;
}

template<typename Coating>
void Material_coating<Coating>::set_coating_normal_map(const Texture_adapter& normal_map) {
	coating_normal_map_ = normal_map;
}

template<typename Coating>
void Material_coating<Coating>::set_coating_weight(float weight) {
	coating_.weight_ = weight;
}

template<typename Coating>
void Material_coating<Coating>::set_coating_color(float3 const& color) {
	coating_.color_ = color;
}

template<typename Coating>
template<typename Sample>
void Material_coating<Coating>::set_coating_basis(f_float3 wo, const Renderstate& rs,
												  const Texture_sampler_2D& sampler,
												  Sample& sample) const {
	if (Material_base::normal_map_ == coating_normal_map_) {
		sample.coating_.set_tangent_frame(sample.layer_.t_, sample.layer_.b_, sample.layer_.n_);
	} else if (coating_normal_map_.is_valid()) {
		float3 const n = sample_normal(wo, rs, normal_map_, sampler);
		sample.coating_.set_tangent_frame(n);
	} else {
		sample.coating_.set_tangent_frame(rs.t, rs.b, rs.n);
	}

	if (coating_weight_map_.is_valid()) {
		float weight = coating_weight_map_.sample_1(sampler, rs.uv);
		sample.coating_.set_color_and_weight(coating_.color_, weight);
	} else {
		sample.coating_.set_color_and_weight(coating_.color_, coating_.weight_);
	}
}

}
