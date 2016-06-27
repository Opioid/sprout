#pragma once

#include "substitute_coating_material.hpp"
#include "substitute_base_material.inl"

namespace scene { namespace material { namespace substitute {

template<typename Coating, typename Sample>
Material_coating<Coating, Sample>::Material_coating(
		Generic_sample_cache<Sample>& cache, Texture_2D_ptr mask,
		const Sampler_settings& sampler_settings, bool two_sided) :
	Material_base<Sample>(cache, mask, sampler_settings, two_sided) {}

template<typename Coating, typename Sample>
void Material_coating<Coating, Sample>::set_coating_normal_map(Texture_2D_ptr normal_map) {
	coating_normal_map_ = normal_map;
}

template<typename Coating, typename Sample>
void Material_coating<Coating, Sample>::set_coating_basis(const Renderstate& rs,
														  const Texture_sampler_2D& sampler,
														  Sample& sample) {
	if (normal_map_ == coating_normal_map_) {
		sample.coating_.set_basis(rs.t, rs.b, sample.layer_.n);
	} else if (coating_normal_map_) {
		float3 nm = sampler.sample_3(*coating_normal_map_, rs.uv);
		float3 n  = math::normalized(rs.tangent_to_world(nm));

		sample.coating_.set_basis(rs.t, rs.b, n);
	} else {
		sample.coating_.set_basis(rs.t, rs.b, rs.n);
	}
}

}}}
