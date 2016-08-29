#include "glass_rough_material.hpp"
#include "glass_rough_sample.hpp"
#include "image/texture/texture_2d_adapter.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.hpp"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material_sample.inl"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/hitpoint.inl"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace glass {

Glass_rough::Glass_rough(Generic_sample_cache<Sample_rough>& cache,
						 const Sampler_settings& sampler_settings) :
	Typed_material(cache, sampler_settings, false) {}

const material::Sample& Glass_rough::sample(float3_p wo, const Renderstate& rs,
											const Worker& worker, Sampler_filter filter) {
	auto& sample = cache_.get(worker.id());

	sample.set_basis(rs.geo_n, wo);

	auto& sampler = worker.sampler(sampler_key_, filter);

	if (normal_map_.is_valid()) {
		float3 nm = normal_map_.sample_3(sampler, rs.uv);
		float3 n  = math::normalized(rs.tangent_to_world(nm));

		sample.layer_.set_basis(rs.t, rs.b, n);
	} else {
		sample.layer_.set_basis(rs.t, rs.b, rs.n);
	}

	float a2;
	if (roughness_map_.is_valid()) {
		float roughness = ggx::map_roughness(roughness_map_.sample_1(sampler, rs.uv));
		a2 = math::pow4(roughness);
	} else {
		a2 = a2_;
	}

	sample.layer_.set(color_, attenuation_distance_, ior_, rs.ior, a2);

	return sample;
}

void Glass_rough::set_normal_map(const Adapter_2D& normal_map) {
	normal_map_ = normal_map;
}

void Glass_rough::set_roughness_map(const Adapter_2D& roughness_map) {
	roughness_map_ = roughness_map;
}

void Glass_rough::set_color(float3_p color) {
	color_ = color;
}

void Glass_rough::set_attenuation_distance(float attenuation_distance) {
	attenuation_distance_ = attenuation_distance;
}

void Glass_rough::set_ior(float ior) {
	ior_ = ior;
}

void Glass_rough::set_roughness(float roughness) {
	a2_ = math::pow4(ggx::clamp_roughness(roughness));
}

}}}
