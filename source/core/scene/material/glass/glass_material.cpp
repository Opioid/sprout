#include "glass_material.hpp"
#include "glass_sample.hpp"
#include "image/texture/texture_2d_adapter.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/hitpoint.inl"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace glass {

Glass::Glass(Generic_sample_cache<Sample>& cache,
			 const Sampler_settings& sampler_settings) :
	Typed_material(cache, sampler_settings, false) {}

const material::Sample& Glass::sample(float3_p wo, const Renderstate& rs,
									  const Worker& worker, Sampler_filter filter) {
	auto& sample = cache_.get(worker.id());

	sample.set_basis(rs.geo_n, wo);

	if (normal_map_.is_valid()) {
		auto& sampler = worker.sampler(sampler_key_, filter);

		float3 nm = normal_map_.sample_3(sampler, rs.uv);
		float3 n  = math::normalized(rs.tangent_to_world(nm));

		sample.layer_.set_basis(rs.t, rs.b, n);
	} else {
		sample.layer_.set_basis(rs.t, rs.b, rs.n);
	}

	sample.layer_.set(color_, attenuation_distance_, ior_, rs.ior);

	return sample;
}

void Glass::set_normal_map(const Adapter_2D& normal_map) {
	normal_map_ = normal_map;
}

void Glass::set_color(const float3& color) {
	color_ = color;
}

void Glass::set_attenuation_distance(float attenuation_distance) {
	attenuation_distance_ = attenuation_distance;
}

void Glass::set_ior(float ior) {
	ior_ = ior;
}

}}}
