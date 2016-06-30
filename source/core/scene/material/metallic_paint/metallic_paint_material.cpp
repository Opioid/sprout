#include "metallic_paint_material.hpp"
#include "metallic_paint_sample.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.hpp"
#include "image/texture/sampler/sampler_2d.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/hitpoint.inl"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace metallic_paint {

Material::Material(Generic_sample_cache<Sample>& cache,
				   const Sampler_settings& sampler_settings, bool two_sided) :
	material::Typed_material<Generic_sample_cache<Sample>>(cache, sampler_settings,
														   two_sided) {}

const material::Sample& Material::sample(float3_p wo, const Renderstate& rs,
										 const Worker& worker,
										 Sampler_settings::Filter filter) {
	auto& sample = cache_.get(worker.id());

	sample.set_basis(rs.geo_n, wo);

	if (normal_map_) {
		auto& sampler = worker.sampler(sampler_key_, filter);

		float3 nm = sampler.sample_3(*normal_map_, rs.uv);
		float3 n  = math::normalized(rs.tangent_to_world(nm));
		sample.layer_.set_basis(rs.t, rs.b, n);
	} else {
		sample.layer_.set_basis(rs.t, rs.b, rs.n);
	}

	sample.layer_.set(ior_, absorption_, roughness_);

	return sample;
}

void Material::set_normal_map(Texture_2D_ptr normal_map) {
	normal_map_ = normal_map;
}

void Material::set_ior(float3_p ior) {
	ior_ = ior;
}

void Material::set_absorption(float3_p absorption) {
	absorption_ = absorption;
}

void Material::set_roughness(float roughness) {
	roughness_ = roughness;
}

}}}
