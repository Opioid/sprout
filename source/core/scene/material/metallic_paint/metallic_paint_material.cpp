#include "metallic_paint_material.hpp"
#include "metallic_paint_sample.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.hpp"
#include "image/texture/texture_2d_adapter.inl"
#include "scene/material/material_sample.inl"
#include "scene/material/material_sample_cache.inl"
#include "scene/material/coating/coating.inl"
#include "scene/shape/geometry/hitpoint.inl"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace metallic_paint {

Material::Material(Generic_sample_cache<Sample>& cache,
				   const Sampler_settings& sampler_settings, bool two_sided) :
	material::Typed_material<Generic_sample_cache<Sample>>(cache, sampler_settings, two_sided) {}

const material::Sample& Material::sample(float3_p wo, const Renderstate& rs,
										 const Worker& worker, Sampler_filter filter) {
	auto& sample = cache_.get(worker.id());

	sample.set_basis(rs.geo_n, wo);
/*
	if (normal_map_) {
		auto& sampler = worker.sampler(sampler_key_, filter);

		float3 nm = sampler.sample_3(*normal_map_, rs.uv);
		float3 n  = math::normalized(rs.tangent_to_world(nm));
		sample.layer_.set_basis(rs.t, rs.b, n);
	} else {*/
		sample.base_.set_basis(rs.t, rs.b, rs.n);

		sample.coating_.set_basis(rs.t, rs.b, rs.n);
//	}

	auto& sampler = worker.sampler(sampler_key_, filter);

	if (flakes_normal_map_.is_valid()) {

		float3 nm = flakes_normal_map_.sample_3(sampler, rs.uv);
		float3 n = math::normalized(rs.tangent_to_world(nm));

		sample.flakes_.set_basis(rs.t, rs.b, n);
	} else {
		sample.flakes_.set_basis(rs.t, rs.b, rs.n);
	}

	sample.base_.set(color_a_, color_b_);

	if (flakes_mask_.is_valid()) {
		float weight = flakes_mask_.sample_1(sampler, rs.uv);
		sample.flakes_.weight = weight;
	} else {
		sample.flakes_.weight = 1.f;
	}

	sample.flakes_.ior = float3(0.18267f, 0.49447f, 1.3761f);
	sample.flakes_.absorption = float3(3.1178f, 2.3515f, 1.8324f);
	sample.flakes_.a2 = math::pow4(0.15f);

	sample.coating_.set_color_and_weight(coating_.color, coating_.weight);

	sample.coating_.set(coating_.f0, coating_.a2);

	return sample;
}

void Material::set_flakes_mask(const Adapter_2D& mask) {
	flakes_mask_ = mask;
}

void Material::set_flakes_normal_map(const Adapter_2D& normal_map) {
	flakes_normal_map_ = normal_map;
}

void Material::set_color(float3_p a, float3_p b) {
	color_a_ = a;
	color_b_ = b;
}

void Material::set_coating_weight(float weight) {
	coating_.weight = weight;
}

void Material::set_coating_color(float3_p color) {
	coating_.color = color;
}

void Material::set_clearcoat(float ior, float roughness) {
	coating_.f0 = fresnel::schlick_f0(1.f, ior);
	coating_.a2 = math::pow4(roughness);
}

}}}
